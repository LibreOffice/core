#!/bin/sh

# use gnome-open utility coming with libgnome if available
gnome-open "$1" 2>/dev/null || "$0.bin" $1

exit 0
