#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <iomanip>
#include <climits>

using namespace std;

struct Process {
    int pid;         // Process ID
    int arrival;     // Arrival time
    int burst;       // Original burst time
    int remaining;   // Remaining burst time
    int priority;    // Priority level
    int waiting;     // Waiting time
    int turnaround;  // Turnaround time
    int response;   // Response time
    int lastRun;     // Last time process was run
    int queueLevel;  // For Feedback algorithm
    bool started;    // To track if process has started
};

// Function prototypes
void FCFS(vector<Process>& processes);
void RoundRobin(vector<Process>& processes, int quantum);
void SPN(vector<Process>& processes);
void SRT(vector<Process>& processes);
void HRRN(vector<Process>& processes);
void Feedback(vector<Process>& processes, int quantum);
void Aging(vector<Process>& processes, int quantum);
void printResults(const vector<Process>& processes);
void resetProcesses(vector<Process>& processes);

int main() {
    vector<Process> processes = {
        {1, 0, 10, 10, 3, 0, 0, -1, -1, 0, false},
        {2, 1, 5, 5, 1, 0, 0, -1, -1, 0, false},
        {3, 2, 8, 8, 2, 0, 0, -1, -1, 0, false},
        {4, 3, 2, 2, 4, 0, 0, -1, -1, 0, false},
        {5, 4, 7, 7, 5, 0, 0, -1, -1, 0, false}
    };

    int choice;
    int quantum;

    do {
        cout << "\nCPU Scheduling Algorithms Simulator\n";
        cout << "--------------------------------\n";
        cout << "1. FCFS (First-Come First-Served)\n";
        cout << "2. Round Robin\n";
        cout << "3. SPN (Shortest Process Next)\n";
        cout << "4. SRT (Shortest Remaining Time)\n";
        cout << "5. HRRN (Highest Response Ratio Next)\n";
        cout << "6. Feedback\n";
        cout << "7. Aging\n";
        cout << "8. Exit\n";
        cout << "Select algorithm: ";
        cin >> choice;

        if (choice >= 2 && choice <= 7 && choice != 5) {
            cout << "Enter time quantum: ";
            cin >> quantum;
        }

        resetProcesses(processes);

        switch (choice) {
            case 1:
                FCFS(processes);
                break;
            case 2:
                RoundRobin(processes, quantum);
                break;
            case 3:
                SPN(processes);
                break;
            case 4:
                SRT(processes);
                break;
            case 5:
                HRRN(processes);
                break;
            case 6:
                Feedback(processes, quantum);
                break;
            case 7:
                Aging(processes, quantum);
                break;
            case 8:
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice!\n";
        }

        if (choice >= 1 && choice <= 7) {
            printResults(processes);
        }

    } while (choice != 8);

    return 0;
}

void resetProcesses(vector<Process>& processes) {
    for (auto& p : processes) {
        p.remaining = p.burst;
        p.waiting = 0;
        p.turnaround = 0;
        p.response = -1;
        p.lastRun = -1;
        p.queueLevel = 0;
        p.started = false;
    }
}

void printResults(const vector<Process>& processes) {
    cout << "\nProcess ID | Arrival | Burst | Priority | Waiting | Turnaround | Response\n";
    cout << "------------------------------------------------------------------------\n";

    float avg_waiting = 0, avg_turnaround = 0, avg_response = 0;

    for (const auto& p : processes) {
        cout << setw(6) << p.pid << "   |"
             << setw(8) << p.arrival << " |"
             << setw(6) << p.burst << " |"
             << setw(9) << p.priority << " |"
             << setw(8) << p.waiting << " |"
             << setw(11) << p.turnaround << " |"
             << setw(9) << p.response << endl;

        avg_waiting += p.waiting;
        avg_turnaround += p.turnaround;
        avg_response += p.response;
    }

    avg_waiting /= processes.size();
    avg_turnaround /= processes.size();
    avg_response /= processes.size();

    cout << fixed << setprecision(2);
    cout << "\nAverage Waiting Time: " << avg_waiting << endl;
    cout << "Average Turnaround Time: " << avg_turnaround << endl;
    cout << "Average Response Time: " << avg_response << endl;
}

void FCFS(vector<Process>& processes) {
    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), 
        [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

    int currentTime = 0;
    
    for (auto& p : processes) {
        if (currentTime < p.arrival) {
            currentTime = p.arrival;
        }
        
        p.response = currentTime - p.arrival;
        p.waiting = currentTime - p.arrival;
        currentTime += p.burst;
        p.turnaround = currentTime - p.arrival;
        p.remaining = 0;
    }
}

void RoundRobin(vector<Process>& processes, int quantum) {
    queue<int> readyQueue;
    vector<bool> isQueued(processes.size() + 1, false);
    int currentTime = 0;
    int completed = 0;
    int n = processes.size();

    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), 
        [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

    readyQueue.push(0);
    isQueued[0] = true;

    while (completed < n) {
        if (readyQueue.empty()) {
            currentTime++;
            // Check for new arrivals
            for (int i = 0; i < n; i++) {
                if (!isQueued[i] && processes[i].arrival <= currentTime && processes[i].remaining > 0) {
                    readyQueue.push(i);
                    isQueued[i] = true;
                    break;
                }
            }
            continue;
        }

        int idx = readyQueue.front();
        readyQueue.pop();
        isQueued[idx] = false;

        Process& p = processes[idx];

        if (!p.started) {
            p.response = currentTime - p.arrival;
            p.started = true;
        }

        int executionTime = min(quantum, p.remaining);
        p.remaining -= executionTime;
        currentTime += executionTime;

        // Check for new arrivals during this execution
        for (int i = 0; i < n; i++) {
            if (!isQueued[i] && processes[i].arrival <= currentTime && 
                processes[i].remaining > 0 && i != idx) {
                readyQueue.push(i);
                isQueued[i] = true;
            }
        }

        if (p.remaining > 0) {
            readyQueue.push(idx);
            isQueued[idx] = true;
        } else {
            completed++;
            p.turnaround = currentTime - p.arrival;
            p.waiting = p.turnaround - p.burst;
        }
    }
}

void SPN(vector<Process>& processes) {
    sort(processes.begin(), processes.end(), 
        [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

    int currentTime = 0;
    int completed = 0;
    int n = processes.size();
    vector<bool> completedProcesses(n, false);

    while (completed < n) {
        int shortest = -1;
        int shortestBurst = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (!completedProcesses[i] && processes[i].arrival <= currentTime && 
                processes[i].burst < shortestBurst) {
                shortest = i;
                shortestBurst = processes[i].burst;
            }
        }

        if (shortest == -1) {
            currentTime++;
            continue;
        }

        Process& p = processes[shortest];
        p.response = currentTime - p.arrival;
        p.waiting = currentTime - p.arrival;
        currentTime += p.burst;
        p.turnaround = currentTime - p.arrival;
        p.remaining = 0;
        completedProcesses[shortest] = true;
        completed++;
    }
}

void SRT(vector<Process>& processes) {
    sort(processes.begin(), processes.end(), 
        [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

    int currentTime = 0;
    int completed = 0;
    int n = processes.size();
    int prevProcess = -1;

    while (completed < n) {
        int shortest = -1;
        int shortestRemaining = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival <= currentTime && processes[i].remaining > 0 && 
                processes[i].remaining < shortestRemaining) {
                shortest = i;
                shortestRemaining = processes[i].remaining;
            }
        }

        if (shortest == -1) {
            currentTime++;
            continue;
        }

        Process& p = processes[shortest];

        if (!p.started) {
            p.response = currentTime - p.arrival;
            p.started = true;
        }

        // Update waiting time for other processes
        for (int i = 0; i < n; i++) {
            if (i != shortest && processes[i].arrival <= currentTime && 
                processes[i].remaining > 0) {
                processes[i].waiting++;
            }
        }

        p.remaining--;
        currentTime++;

        if (p.remaining == 0) {
            completed++;
            p.turnaround = currentTime - p.arrival;
        }
    }
}

void HRRN(vector<Process>& processes) {
    sort(processes.begin(), processes.end(), 
        [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

    int currentTime = 0;
    int completed = 0;
    int n = processes.size();
    vector<bool> completedProcesses(n, false);

    while (completed < n) {
        int highest = -1;
        float highestRatio = -1.0f;

        for (int i = 0; i < n; i++) {
            if (!completedProcesses[i] && processes[i].arrival <= currentTime) {
                float ratio = (float)(currentTime - processes[i].arrival + processes[i].burst) / processes[i].burst;
                if (ratio > highestRatio) {
                    highest = i;
                    highestRatio = ratio;
                }
            }
        }

        if (highest == -1) {
            currentTime++;
            continue;
        }

        Process& p = processes[highest];
        p.response = currentTime - p.arrival;
        p.waiting = currentTime - p.arrival;
        currentTime += p.burst;
        p.turnaround = currentTime - p.arrival;
        p.remaining = 0;
        completedProcesses[highest] = true;
        completed++;
    }
}

void Feedback(vector<Process>& processes, int quantum) {
    sort(processes.begin(), processes.end(), 
        [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

    vector<queue<int>> queues(5); // 5 priority levels
    vector<bool> isCompleted(processes.size(), false);
    int currentTime = 0;
    int completed = 0;
    int n = processes.size();

    // Initial queue assignment
    for (int i = 0; i < n; i++) {
        if (processes[i].arrival <= currentTime) {
            queues[0].push(i);
        }
    }

    while (completed < n) {
        bool found = false;
        
        // Check queues from highest to lowest priority
        for (int q = 0; q < queues.size(); q++) {
            if (!queues[q].empty()) {
                int idx = queues[q].front();
                queues[q].pop();
                found = true;
                
                Process& p = processes[idx];
                
                if (!p.started) {
                    p.response = currentTime - p.arrival;
                    p.started = true;
                }

                int executionTime = min(quantum * (q + 1), p.remaining);
                p.remaining -= executionTime;
                currentTime += executionTime;

                // Check for new arrivals during this execution
                for (int i = 0; i < n; i++) {
                    if (processes[i].arrival <= currentTime && processes[i].remaining > 0 && 
                        !isCompleted[i] && i != idx) {
                        bool alreadyInQueue = false;
                        for (const auto& queue : queues) {
                            // This is simplified - in real implementation would need to check all elements
                            if (!queue.empty() && queue.front() == i) {
                                alreadyInQueue = true;
                                break;
                            }
                        }
                        if (!alreadyInQueue) {
                            queues[0].push(i);
                        }
                    }
                }

                if (p.remaining > 0) {
                    // Demote to next lower priority queue
                    if (q + 1 < queues.size()) {
                        queues[q + 1].push(idx);
                    } else {
                        queues.back().push(idx);
                    }
                } else {
                    completed++;
                    isCompleted[idx] = true;
                    p.turnaround = currentTime - p.arrival;
                    p.waiting = p.turnaround - p.burst;
                }
                
                break;
            }
        }
        
        if (!found) {
            currentTime++;
            // Check for new arrivals
            for (int i = 0; i < n; i++) {
                if (processes[i].arrival <= currentTime && processes[i].remaining > 0 && 
                    !isCompleted[i]) {
                    bool alreadyInQueue = false;
                    for (const auto& queue : queues) {
                        // Simplified check
                        if (!queue.empty() && queue.front() == i) {
                            alreadyInQueue = true;
                            break;
                        }
                    }
                    if (!alreadyInQueue) {
                        queues[0].push(i);
                    }
                }
            }
        }
    }
}

void Aging(vector<Process>& processes, int quantum) {
    sort(processes.begin(), processes.end(), 
        [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

    vector<queue<int>> priorityQueues(5); // 5 priority levels
    vector<int> lastRunTime(processes.size(), 0);
    vector<bool> isCompleted(processes.size(), false);
    int currentTime = 0;
    int completed = 0;
    int n = processes.size();

    // Initial queue assignment based on priority
    for (int i = 0; i < n; i++) {
        if (processes[i].arrival <= currentTime) {
            int queueLevel = min(4, max(0, 4 - processes[i].priority));
            priorityQueues[queueLevel].push(i);
        }
    }

    while (completed < n) {
        bool found = false;
        
        // Check queues from highest to lowest priority
        for (int q = 0; q < priorityQueues.size(); q++) {
            if (!priorityQueues[q].empty()) {
                int idx = priorityQueues[q].front();
                priorityQueues[q].pop();
                found = true;
                
                Process& p = processes[idx];
                
                if (!p.started) {
                    p.response = currentTime - p.arrival;
                    p.started = true;
                }

                int executionTime = min(quantum, p.remaining);
                p.remaining -= executionTime;
                currentTime += executionTime;
                lastRunTime[idx] = currentTime;

                // Aging: increase priority of waiting processes
                for (int i = 0; i < n; i++) {
                    if (i != idx && processes[i].arrival <= currentTime && 
                        processes[i].remaining > 0 && !isCompleted[i]) {
                        int waitTime = currentTime - lastRunTime[i];
                        if (waitTime >= quantum * 2) { // Aging threshold
                            int currentQueue = min(4, max(0, 4 - processes[i].priority));
                            if (currentQueue > 0) {
                                // Promote to higher priority queue
                                priorityQueues[currentQueue - 1].push(i);
                                lastRunTime[i] = currentTime;
                                // Need to remove from lower priority queue if present
                                // (This is simplified - would need more complex tracking)
                            }
                        }
                    }
                }

                // Check for new arrivals during this execution
                for (int i = 0; i < n; i++) {
                    if (processes[i].arrival <= currentTime && processes[i].remaining > 0 && 
                        !isCompleted[i] && i != idx) {
                        bool alreadyInQueue = false;
                        for (const auto& queue : priorityQueues) {
                            // Simplified check
                            if (!queue.empty() && queue.front() == i) {
                                alreadyInQueue = true;
                                break;
                            }
                        }
                        if (!alreadyInQueue) {
                            int queueLevel = min(4, max(0, 4 - processes[i].priority));
                            priorityQueues[queueLevel].push(i);
                        }
                    }
                }

                if (p.remaining > 0) {
                    // Requeue at same priority level
                    priorityQueues[q].push(idx);
                } else {
                    completed++;
                    isCompleted[idx] = true;
                    p.turnaround = currentTime - p.arrival;
                    p.waiting = p.turnaround - p.burst;
                }
                
                break;
            }
        }
        
        if (!found) {
            currentTime++;
            // Check for new arrivals
            for (int i = 0; i < n; i++) {
                if (processes[i].arrival <= currentTime && processes[i].remaining > 0 && 
                    !isCompleted[i]) {
                    bool alreadyInQueue = false;
                    for (const auto& queue : priorityQueues) {
                        // Simplified check
                        if (!queue.empty() && queue.front() == i) {
                            alreadyInQueue = true;
                            break;
                        }
                    }
                    if (!alreadyInQueue) {
                        int queueLevel = min(4, max(0, 4 - processes[i].priority));
                        priorityQueues[queueLevel].push(i);
                    }
                }
            }
        }
    }
}
