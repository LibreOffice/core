#!/bin/bash -e
#
# './g pull -r' just forwards to 'git pull -r'.
#
# './g review' to submit changes for review on Collabora's Gerrit, assuming:
# 1) You have a remote pointing at ssh://<user>@gerrit.collaboraoffice.com:29418/online
#    (typically named 'origin' or 'cogerrit').
# 2) All commits but the topmost are pushed as WIP; the topmost commit is the
#    actual review.
#

# e.g. co-4-2
BRANCH=$(git symbolic-ref HEAD|sed 's|refs/heads/||')
# e.g. origin
REMOTE=$(git config branch.$BRANCH.remote)

if [ "$1" == "review" ]; then
    # Find the remote that points at Collabora's Gerrit.  This is
    # usually 'origin' or 'cogerrit' depending on user's setup.
    GERRIT_REMOTE=
    for r in $(git remote); do
        if git remote get-url "$r" | grep -q gerrit.collaboraoffice.com; then
            GERRIT_REMOTE=$r
            break
        fi
    done
    if [ -z "$GERRIT_REMOTE" ]; then
        echo "Error: no remote pointing at gerrit.collaboraoffice.com found."
        echo "Add one with e.g.:"
        echo "  git remote add cogerrit ssh://\$USER@gerrit.collaboraoffice.com:29418/online"
        exit 1
    fi

    # The branch we're pushing changes for, derived from the upstream
    # branch (e.g. main, distro/collabora/co-26-04).
    TRACKED_BRANCH=$(git rev-parse --abbrev-ref --symbolic-full-name HEAD@{upstream}|sed "s|${REMOTE}/||")

    # Push everything except the topmost commit as WIP, and the
    # topmost commit as the actual review.
    NUM_COMMITS=$(git rev-list --count $REMOTE/$TRACKED_BRANCH..HEAD)
    if [ "$NUM_COMMITS" -lt 1 ]; then
        echo "Error: no commits to push for review."
        exit 1
    fi
    if [ "$NUM_COMMITS" -gt 1 ]; then
        git push $GERRIT_REMOTE HEAD~1:refs/for/$TRACKED_BRANCH%wip
    fi
    git push $GERRIT_REMOTE HEAD:refs/for/$TRACKED_BRANCH

    exit 0
fi

if [ "$1" == "pull" ]; then
    shift
    git pull "$@"
    exit 0
fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
