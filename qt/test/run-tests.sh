#!/usr/bin/env bash
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Headless test runner for coda-qt WebdriverIO tests.
#
# Usage:
#   AT_SPI_DRIVER_PATH=/path/to/selenium-webdriver-at-spi.py ./run-tests.sh
#
# Environment:
#   AT_SPI_DRIVER_PATH  (required) Path to selenium-webdriver-at-spi.py
#   CODA_QT_BINARY      Path to coda-qt (default: ../coda-qt)
#   AT_SPI_VENV         Path to the AT-SPI driver virtualenv (default: <AT_SPI_DRIVER_PATH>/venv)
#   CODA_QT_TEST_GUI    Set to 1 to run on the host display instead of a virtual Wayland compositor.
#
# Extra args after the flags are forwarded to `wdio run` (e.g. --suite gui).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ -z "${AT_SPI_DRIVER_PATH:-}" ]; then
    echo "ERROR: AT_SPI_DRIVER_PATH is not set."
    echo "  export AT_SPI_DRIVER_PATH=/path/to/selenium-webdriver-at-spi.py"
    exit 1
fi
export AT_SPI_DRIVER_PATH
ATSPI_DIR="$(dirname "$AT_SPI_DRIVER_PATH")"

AT_SPI_VENV="${AT_SPI_VENV:-$ATSPI_DIR/venv}"
if [ -d "$AT_SPI_VENV/bin" ]; then
    export PATH="$AT_SPI_VENV/bin:$PATH"
fi

check_cmd() { command -v "$1" >/dev/null 2>&1 || { echo "ERROR: $1 not found. $2"; exit 1; }; }
check_cmd weston        "Install weston."
check_cmd qtpaths6      "Install Qt 6 development tools (qt6-qtbase-devel)."
check_cmd flask         "Run: pip install flask (inside the AT-SPI venv)."
check_cmd node          "Install Node.js >= 18."

# Re-exec under setsid so all child processes share one process group.
# The outer shell kills the entire group on exit
if [ "${_CODA_QT_TEST_ISOLATED:-}" != "1" ]; then
    _CODA_QT_TEST_ISOLATED=1 setsid "$0" "$@" &
    INNER_PID=$!
    trap 'kill -- -"$INNER_PID" 2>/dev/null; wait "$INNER_PID" 2>/dev/null || true' EXIT
    RC=0; wait "$INNER_PID" || RC=$?
    exit "$RC"
fi

WAYLAND_SOCKET="coda-qt-test-$$"
WESTON_PID=""

cleanup() {
    [ -n "$WESTON_PID" ] && kill "$WESTON_PID" 2>/dev/null && wait "$WESTON_PID" 2>/dev/null || true
}
trap cleanup EXIT

run_tests() {
    [ -z "${CODA_QT_TEST_GUI:-}" ] && export LIBGL_ALWAYS_SOFTWARE=1
    export QT_QPA_PLATFORM=wayland
    export XKB_DEFAULT_LAYOUT=us

    local weston_args=(--no-config --socket="$WAYLAND_SOCKET")
    [ -z "${CODA_QT_TEST_GUI:-}" ] && weston_args+=(--backend=headless)

    weston "${weston_args[@]}" &>/dev/null &
    WESTON_PID=$!

    # Wait for weston to create the wayland socket.
    local socket_path="${XDG_RUNTIME_DIR:-/run/user/$(id -u)}/$WAYLAND_SOCKET"
    for _i in {1..30}; do
        [ -S "$socket_path" ] && break
        sleep 0.2
    done
    if [ ! -S "$socket_path" ]; then
        echo "ERROR: Wayland socket $socket_path not created"; exit 1
    fi
    export WAYLAND_DISPLAY="$WAYLAND_SOCKET"

    cd "$SCRIPT_DIR"
    npx wdio run wdio.conf.ts "$@"
}

if [ -n "${DBUS_SESSION_BUS_ADDRESS:-}" ]; then
    run_tests "$@"
else
    # run_tests needs access to WESTON_PID for cleanup, so exec into
    # dbus-run-session rather than spawning a subprocess.
    exec dbus-run-session -- "$0" "$@"
fi
