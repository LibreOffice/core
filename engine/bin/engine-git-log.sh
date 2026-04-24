#!/bin/sh
# Show the full log for a file across the engine/ subtree merge.
# Usage: engine-git-log.sh <path>
# Path may be given relative to the current directory or from the
# repo root, with or without the engine/ prefix.
prefix=$(git rev-parse --show-prefix) || exit 1
cd "$(git rev-parse --show-toplevel)" || exit 1
new=${prefix}$1
old=${new#engine/}
if [ -t 1 ]; then pager=$(git var GIT_PAGER); else pager=cat; fi
{
    git -P log --follow merge-engine..HEAD -- "$new"
    git -P log --follow merge-engine^2     -- "$old"
} | $pager
