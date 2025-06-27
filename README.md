
## Documentation 

### Safety-Conscious-Linux

This code is a safety-conscious Linux program designed to:

1. **Set real-time scheduling priority**
2. **Read sensor data using ioctl**
3. **Ping a watchdog device to prevent system reset**
4. **Log all critical errors using syslog**



---

###  Preprocessor & Includes

```c
#define _GNU_SOURCE
```
Ensures GNU-specific functions like `sched_setscheduler` behave as expected.

```c
#include <...>
```
Includes standard C headers for I/O, file control, scheduling, errors, and system logging.

---

###  Device and IOCTL Macros

```c
#define SENSOR_DEV "/dev/sensor0"
#define WATCHDOG_DEV "/dev/watchdog"
#define SENSOR_IOCTL_READ _IOR('s', 0x01, int)
#define WATCHDOG_PING_IOCTL 0x80045705
```

- `SENSOR_DEV`: Represents a custom sensor device node.
- `WATCHDOG_DEV`: Refers to the Linux watchdog API interface.
- `SENSOR_IOCTL_READ`: Uses the `_IOR` macro to build a unique IOCTL request.
- `WATCHDOG_PING_IOCTL`: Used to ping the watchdog and prevent system reset (would normally be `WDIOC_KEEPALIVE`).

---

### ️ set_realtime_priority()

```c
sched.sched_priority = 20;
sched_setscheduler(0, SCHED_FIFO, &sched);
```

Requests that the process runs with real-time scheduling using the `SCHED_FIFO` algorithm and priority 20. This ensures time-sensitive operations (e.g., sensor polling) are not delayed by other processes.

---

###  read_sensor()

```c
ioctl(sensor_fd, SENSOR_IOCTL_READ, value);
```

Attempts to read from the sensor using a custom `ioctl`. If it fails, it logs the error and returns.

---

###  ping_watchdog()

```c
ioctl(wd_fd, WATCHDOG_PING_IOCTL, &dummy);
```

Sends a keep-alive signal to the watchdog device, preventing the system from rebooting if the app is still healthy.

---

###  main()

1. Initializes system logging with `openlog(LOG_TAG, ...)`.
2. Sets real-time priority.
3. Opens the sensor and watchdog devices (with error handling).
4. Calls `read_sensor()` to get the sensor value.
5. Outputs that value.
6. Sends a keep-alive ping to the watchdog.
7. Closes both devices and logs any errors.
8. Closes the syslog and exits cleanly.

---

###  Safety-Critical Features

- Strict checking of all return values
- Graceful shutdown on error
- Log messages for every failure point
- No dynamic memory allocation
- Suitable foundation for watchdog-controlled embedded Linux services

---


###  Example Bash Commands

Compile your code into an executable called e.g. `safety_app`.

```bash
#!/bin/bash
# safety.sh — Bash wrapper for safety_app

APP="./safety_app"

read_sensor() {
    echo "Reading sensor value..."
    $APP read
}

ping_watchdog() {
    echo "Pinging watchdog..."
    $APP ping
}

read_and_ping() {
    echo "Reading sensor and pinging watchdog..."
    $APP both
}

loop_mode() {
    echo "Entering monitor loop (Ctrl+C to stop)..."
    $APP loop
}

usage() {
    echo "Usage: $0 {read|ping|both|loop}"
}

case "$1" in
    read)         read_sensor ;;
    ping)         ping_watchdog ;;
    both)         read_and_ping ;;
    loop)         loop_mode ;;
    *)            usage ;;
esac
```

Make it executable:

```bash
chmod +x safety.sh
```

Now you can run:

```bash
./safety.sh read     # One-time sensor reading
./safety.sh loop     # Continuous monitoring
```

---

###  Add to Cron for Periodic Watchdog Kick

Edit your crontab with:

```bash
crontab -e
```

Then add a line like:

```cron
* * * * * /path/to/safety.sh both >> /var/log/safety.log 2>&1
```

This runs it every minute to keep the watchdog happy and record the sensor state.

Want to extend this with email alerts on threshold breaches or systemd service integration? We can totally do that next.


  
