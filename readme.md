# Post Office Simulation

## Description

This program simulates a post office with multiple clerks (`urednik`) and customers (`zakaznik`) using POSIX semaphores and shared memory.

## Compilation

```bash
gcc -o proj2 proj2.c -pthread
```

## Usage

```bash
./proj2 NZ NU TZ TU F
```

- `NZ` — number of customers (≥ 1)
- `NU` — number of clerks (≥ 1)
- `TZ` — max delay before customer enters (0–10000 ms)
- `TU` — max clerk break duration (0–100 ms)
- `F`  — max time before post office closes (0–10000 ms)

## Behavior

- Customers randomly choose one of three service queues.
- Clerks serve waiting customers from available queues.
- When all queues are empty, clerks can take a break.
- After `F` ms, the post office closes. No new customers enter.
- All events are logged to `proj2.out`.

## Synchronization

- `mutex` — controls access to shared queues.
- `output` — protects access to the output file.
- `sem_queue[1-3]` — clerk signals to waiting customer.
- `zakaznik_coming[1-3]` — customer confirms response.

## Output

Logs actions with line numbers, such as:
- Process start
- Queue entry
- Service start and end
- Clerk breaks
- Office closing

## Notes

- Shared memory via `mmap`
- Properly destroys all semaphores and memory on exit
