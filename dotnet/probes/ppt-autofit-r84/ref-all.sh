#!/bin/sh
# The 26.2.4.2 reference for every document named in $1, into $2, two at a time.
set -e
xargs -a "$1" -d '\n' -P 2 -I{} sh -c './ref-render.sh "$1" "$2" || echo "FAILED $1" >&2' _ {} "$2"
