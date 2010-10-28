#!/bin/bash


#    Watches for formula dumps by starmath and renders them
# If starmath is compiled with the macro DEBUG_ENABLE_DUMPASDOT defined.
# shift+enter, in the visual formula editor will make starmath dump a
# graphviz graph of the formula tree structure. This is very useful when
# debugging. The formula is dumped to /tmp/smath-dump.gv-
# This script monitors this file and renders the graph when written,
# after which the formula is displayed using feh.
#
# Usage: Let this script run in the background, e.g. start it in a terminal
# (and don't close the terminal). Then compile starmath with DEBUG_ENABLE_DUMPASDOT
# defined, use the visual formula editor to write something and hit shift+enter.
#
# Note: This won't work on Windows as the /tmp/ folder will be missing.
#
# Author: "Jonas Finnemann Jensen" <jopsen@gmail.com>

touch /tmp/smath-dump.gv;
while inotifywait -q -e close_write /tmp/smath-dump.gv;
do
    dot -Tpng < /tmp/smath-dump.gv > /tmp/smath-dump.png; > /dev/null
    kill `pidof -s feh`; > /dev/null
    feh /tmp/smath-dump.png & > /dev/null
done
