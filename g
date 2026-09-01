#!/bin/bash -e
#
# './g log [-p] file.cxx' to see the entire 'git log [-p] --follow file.cxx'
# across the engine subtree merge (normal 'git log [-p] --follow' just stops
# at the 'merge-engine' commit and does not show anything further).
#
# './g pull -r' just forwards to 'git pull -r'.
#
# './g review [branch]' to submit changes for review on Collabora's Gerrit,
# assuming:
# 1) You have a remote pointing at ssh://<user>@gerrit.collaboraoffice.com:29418/online
#    (typically named 'origin' or 'cogerrit').
# 2) All commits but the topmost are pushed as WIP; the topmost commit is the
#    actual review.
# If [branch] is given (e.g. 'main' or 'distro/collabora/co-26-04'), the
# current branch's upstream is set to <gerrit-remote>/<branch> first.
# When the change targets main, './g review' also compares its base against
# the newest commit that passed the "Tinderbox for online main" Jenkins job,
# and offers to rebase onto that commit first when the two differ.
#

# e.g. co-4-2
BRANCH=$(git symbolic-ref HEAD|sed 's|refs/heads/||')

# Compare the base of the local commits against the newest commit of main
# that passed the "Tinderbox for online main" Jenkins job.  That job builds
# the tip of main once a day, runs the unit tests and all the cypress
# suites, so its last successful commit is a base known to pass the tests
# that most often fail for reasons unrelated to the change under review.
# When the base differs and there is a terminal to ask on, offers to rebase
# the local commits onto the known-good commit before pushing.  A rebase
# that does not complete stops the whole script before anything is pushed.
# Uses REMOTE and TRACKED_BRANCH set by the review handling below.
recommend_known_good_base() {
    local job_url='https://cpci.cbg.collabora.co.uk:8080/job/Tinderbox%20for%20online%20main/lastSuccessfulBuild'

    local console
    if ! console=$(curl -sf --max-time 30 "$job_url/consoleText"); then
        echo "Note: cannot fetch the tinderbox log from Jenkins, skipping the"
        echo "known-good base check."
        return 0
    fi

    # The job updates its checkout of main with 'git pull -r', so the commit
    # it built is the second hash of the "Updating <old>..<new>" line that
    # the pull prints.  When main did not move since the previous run, the
    # pull prints "Already up to date." instead and the log does not name
    # the built commit at all.
    local known_good
    known_good=$(printf '%s\n' "$console" | awk '
        /^\+ git pull -r$/ { in_pull = 1; next }
        in_pull && /^Updating [0-9a-f]+\.\.[0-9a-f]+$/ {
            sub(/^Updating [0-9a-f]+\.\./, ""); print; exit
        }
        in_pull && /^Already up.to.date/ { print "unchanged"; exit }
        in_pull && /^\+ / { exit }
    ')

    if [ "$known_good" == "unchanged" ]; then
        echo "Note: the last tinderbox run found main unchanged since the run"
        echo "before it, so its log does not name the commit it built."
        echo "Skipping the known-good base check."
        return 0
    fi
    if [ -z "$known_good" ]; then
        echo "Note: the tinderbox log does not contain the expected 'git pull -r'"
        echo "output; the Jenkins job script has probably changed and this check"
        echo "needs updating.  Skipping the known-good base check."
        return 0
    fi

    if ! git merge-base --is-ancestor "$known_good" "$REMOTE/$TRACKED_BRANCH" 2>/dev/null; then
        echo "Warning: $REMOTE/$TRACKED_BRANCH does not contain the last known-good"
        echo "commit $known_good.  Your last fetch is older than the last good"
        echo "tinderbox run; run 'git fetch $REMOTE' and consider rebasing onto"
        echo "that commit."
        return 0
    fi

    local base
    base=$(git merge-base HEAD "$REMOTE/$TRACKED_BRANCH")
    if [ "$base" == "$(git rev-parse "$known_good^{commit}" 2>/dev/null)" ]; then
        return 0
    fi

    local when
    when=$(git log -1 --format=%cs "$known_good")
    local count word
    if git merge-base --is-ancestor "$base" "$known_good"; then
        count=$(git rev-list --count "$base..$known_good")
        word=commits; [ "$count" -eq 1 ] && word=commit
        echo "Note: the change is based on a commit $count $word older than the"
        echo "last known-good commit $known_good ($when, the newest commit that"
        echo "passed 'Tinderbox for online main')."
    elif git merge-base --is-ancestor "$known_good" "$base"; then
        count=$(git rev-list --count "$known_good..$base")
        word=commits; [ "$count" -eq 1 ] && word=commit
        echo "Note: the change is based on a commit $count $word newer than the"
        echo "last known-good commit $known_good ($when), so CI will test it on"
        echo "a base that 'Tinderbox for online main' has not verified yet."
    else
        echo "Note: the base of the change and the last known-good commit"
        echo "$known_good ($when, 'Tinderbox for online main') have diverged."
    fi
    # Without a terminal there is nobody to ask, so print the command and
    # push from the current base.
    if [ ! -t 0 ]; then
        echo "To base the change on the known-good commit, run:"
        echo "  git rebase --onto $known_good $REMOTE/$TRACKED_BRANCH"
        echo "No terminal to ask on - pushing from the current base."
        return 0
    fi

    local answer
    while true; do
        echo "How to continue?"
        echo "  1) push anyway (default)"
        echo "  2) rebase onto $known_good and push if the rebase succeeds"
        read -r -p "choice> " answer
        case "$answer" in
            ""|1)
                return 0
                ;;
            2)
                break
                ;;
        esac
    done

    if ! git rebase --onto "$known_good" "$REMOTE/$TRACKED_BRANCH"; then
        echo "Error: the rebase did not complete, nothing was pushed.  Fix the"
        echo "problem git reported above and run './g review' again.  If the"
        echo "rebase stopped half-way, 'git rebase --abort' returns the branch"
        echo "to its state from before the rebase."
        exit 1
    fi
    echo "Rebased onto $known_good."
}

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

    # If a target branch was passed, set the upstream so the rest of the
    # script can derive REMOTE/TRACKED_BRANCH from it normally.
    if [ -n "$2" ]; then
        git branch --set-upstream-to=$GERRIT_REMOTE/$2 $BRANCH
    fi

    # e.g. origin
    REMOTE=$(git config branch.$BRANCH.remote || true)
    if [ -z "$REMOTE" ]; then
        echo "Error: no upstream configured for branch '$BRANCH'."
        echo "Pass a target branch: ./g review <branch>"
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

    # Only main has a tinderbox job wired up here.  The check may rebase
    # the branch, so count the commits to push after it.
    if [ "$TRACKED_BRANCH" == "main" ]; then
        recommend_known_good_base
        NUM_COMMITS=$(git rev-list --count $REMOTE/$TRACKED_BRANCH..HEAD)
    fi

    if [ "$NUM_COMMITS" -gt 1 ]; then
        # Push WIP commits.  If Gerrit rejects with "no new changes" it
        # just means the WIP patches are already up-to-date; continue
        # to push the topmost commit as the actual review.
        set +e
        wip_output=$(git push $GERRIT_REMOTE HEAD~1:refs/for/$TRACKED_BRANCH%wip 2>&1)
        wip_status=$?
        set -e
        if [ $wip_status -eq 0 ]; then
            echo "$wip_output"
        elif echo "$wip_output" | grep -q "no new changes"; then
            echo "Note: old (WIP) commits unchanged on the server - error message suppressed."
        else
            echo "$wip_output"
            exit $wip_status
        fi
    fi
    git push $GERRIT_REMOTE HEAD:refs/for/$TRACKED_BRANCH

    exit 0
fi

if [ "$1" == "pull" ]; then
    shift
    git pull "$@"
    exit 0
fi

if [ "$1" == "log" ]; then
    shift
    "$(dirname -- "${BASH_SOURCE[0]}")"/engine/bin/engine-git-log.sh "$@"
    exit 0
fi

git "$@"

# vim:set shiftwidth=4 softtabstop=4 expandtab:
