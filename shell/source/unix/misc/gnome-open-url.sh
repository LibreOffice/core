#!/bin/sh

# use xdg-open or gnome-open if available, falling back to our own open-url
xdg-open "$1" 2>/dev/null || gnome-open "$1" 2>/dev/null || `dirname "$0"`/open-url "$1" 2>/dev/null
