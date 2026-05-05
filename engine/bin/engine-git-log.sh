#!/bin/sh
# Show the full log for a file across the engine/ subtree merge.
# Usage: engine-git-log.sh [git log args...] <path>
# Path may be given relative to the current directory or from the
# repo root, with or without the engine/ prefix. Any preceding
# arguments are forwarded to git log (e.g. -Sfoo).
if [ $# -lt 1 ]; then
    echo "Usage: $0 [git log args...] <path>" >&2
    exit 1
fi
prefix=$(git rev-parse --show-prefix) || exit 1
cd "$(git rev-parse --show-toplevel)" || exit 1
n=$#
i=0
for arg do
    i=$((i+1))
    if [ $i -eq $n ]; then
        path=$arg
    else
        set -- "$@" "$arg"
    fi
    shift
done
new=${prefix}$path
old=${new#engine/}
if [ -t 1 ]; then
    pager=$(git var GIT_PAGER)
    color=--color=always
else
    pager=cat
    color=
fi
{
    git -P log $color "$@" --follow merge-engine..HEAD -- "$new"
    git -P log $color "$@" --follow merge-engine^2     -- "$old"
} | LESS=${LESS-FRX} LV=${LV--c} $pager
