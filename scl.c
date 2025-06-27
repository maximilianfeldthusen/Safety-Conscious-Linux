
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <syslog.h>

#define SENSOR_DEV "/dev/sensor0"
#define WATCHDOG_DEV "/dev/watchdog"
#define SENSOR_IOCTL_READ _IOR('s', 0x01, int) // Example
#define WATCHDOG_PING_IOCTL 0x80045705         // WDIOC_KEEPALIVE
#define LOG_TAG "safety-app"

int set_realtime_priority(void) {
    struct sched_param sched;
    sched.sched_priority = 20; // POSIX priorities: 1 (low) to 99 (high)
    if (sched_setscheduler(0, SCHED_FIFO, &sched) != 0) {
        perror("sched_setscheduler");
        return -1;
    }
    return 0;
}

int read_sensor(int sensor_fd, int *value) {
    if (ioctl(sensor_fd, SENSOR_IOCTL_READ, value) < 0) {
        syslog(LOG_ERR, "Sensor ioctl failed: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int ping_watchdog(int wd_fd) {
    int dummy = 0;
    if (ioctl(wd_fd, WATCHDOG_PING_IOCTL, &dummy) != 0) {
        syslog(LOG_ERR, "Watchdog ping failed: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int main(void) {
    openlog(LOG_TAG, LOG_PID | LOG_CONS, LOG_USER);

    if (set_realtime_priority() != 0) {
        syslog(LOG_ERR, "Failed to set real-time priority");
        return EXIT_FAILURE;
    }

    int sensor_fd = open(SENSOR_DEV, O_RDONLY);
    if (sensor_fd < 0) {
        syslog(LOG_ERR, "Failed to open sensor: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    int wd_fd = open(WATCHDOG_DEV, O_WRONLY);
    if (wd_fd < 0) {
        syslog(LOG_ERR, "Failed to open watchdog: %s", strerror(errno));
        close(sensor_fd);
        return EXIT_FAILURE;
    }

    int sensor_value = 0;
    if (read_sensor(sensor_fd, &sensor_value) != 0) {
        close(sensor_fd);
        close(wd_fd);
        return EXIT_FAILURE;
    }

    printf("Sensor Value: %d\n", sensor_value);

    if (ping_watchdog(wd_fd) != 0) {
        close(sensor_fd);
        close(wd_fd);
        return EXIT_FAILURE;
    }

    if (close(sensor_fd) != 0 || close(wd_fd) != 0) {
        syslog(LOG_ERR, "Failed to close device file");
        return EXIT_FAILURE;
    }

    closelog();
    return EXIT_SUCCESS;
}

