#!/bin/sh

# use xdg-open or gnome-open if available
xdg-open "$1" 2>/dev/null || gnome-open "$1" 2>/dev/null || "$0.bin" $1

exit 0
