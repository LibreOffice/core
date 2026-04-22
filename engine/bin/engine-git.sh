#!/bin/bash

# Replay a git history-editing subcommand (cherry-pick, revert, ...) from the old core history onto
# the subtree-merged online repo, rewriting all paths under engine/.
#
# The subcommand and its arguments are passed through to git unchanged, so flags like `cherry-pick
# -x` or `revert -m 1` work without this wrapper knowing about them.  The command runs in a
# throwaway detached worktree based at merge-engine^2 (the old core tip at the time of the subtree
# merge), so operations on old-core SHAs succeed there.  The resulting commit(s) are then
# format-patched, path-rewritten, and applied onto the current HEAD with git am.
#
# Usage: engine-git.sh <git-subcommand> [args...]
#   e.g. engine-git.sh cherry-pick 301320d53467
#        engine-git.sh cherry-pick -x 301320d53467
#        engine-git.sh revert 301320d53467

set -euo pipefail

if [ $# -lt 1 ]; then
    printf "usage: %s <git-subcommand> [args...]\n" "$(basename "$0")" >&2
    exit 1
fi

base=merge-engine^2

tmp=$(mktemp -d)
wt="$tmp/wt"
cleanup() {
    git worktree remove --force "$wt" >/dev/null 2>&1 || true
    rm -rf "$tmp"
}
trap cleanup EXIT

git worktree add --detach "$wt" "$base" >/dev/null

pre=$(git -C "$wt" rev-parse HEAD)
git -C "$wt" "$@"
post=$(git -C "$wt" rev-parse HEAD)

if [ "$pre" = "$post" ]; then
    printf "%s produced no commits\n" "$1" >&2
    exit 1
fi

git -C "$wt" format-patch -k --stdout "$pre..$post" \
    | awk '
        /^---$/ { in_diff = 1 }
        in_diff {
            if (/^diff --git a\/.* b\//) {
                sub(/^diff --git a\//, "diff --git a/engine/")
                sub(/ b\//, " b/engine/")
            } else if (/^--- a\//)      { sub(/^--- a\//, "--- a/engine/") }
              else if (/^\+\+\+ b\//)   { sub(/^\+\+\+ b\//, "+++ b/engine/") }
              else if (/^rename from /) { sub(/^rename from /, "rename from engine/") }
              else if (/^rename to /)   { sub(/^rename to /, "rename to engine/") }
              else if (/^copy from /)   { sub(/^copy from /, "copy from engine/") }
              else if (/^copy to /)     { sub(/^copy to /, "copy to engine/") }
        }
        { print }
    ' \
    | git am -k
