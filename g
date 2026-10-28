#!/usr/bin/env bash
set -e

#
# './g log [-p] file.cxx' to see the entire 'git log [-p] --follow file.cxx'
# across the engine subtree merge (normal 'git log [-p] --follow' just stops
# at the 'merge-engine' commit and does not show anything further).
#
# './g pull -r' just forwards to 'git pull -r'.
#
# './g review [branch] [--topic=NAME]' to submit changes for review on
# Collabora's Gerrit, assuming:
# 1) You have a remote pointing at ssh://<user>@gerrit.collaboraoffice.com:29418/online
#    (typically named 'origin' or 'cogerrit').
# 2) All commits but the topmost are pushed as WIP; the topmost commit is the
#    actual review.
# If [branch] is given (e.g. 'main' or 'distro/collabora/co-26-04'), the
# current branch's upstream is set to <gerrit-remote>/<branch> first.
# Pass '--topic=NAME' (or '-t NAME') to set the Gerrit topic of every change
# the push creates or updates, WIP commits included.
# When the change targets main, './g review' also compares its base against
# the newest commit that passed the "Tinderbox for online main" Jenkins job,
# and offers to rebase onto that commit first when the two differ.
# Commits Gerrit already has (checked anonymously over HTTPS) are not pushed
# again; when it has HEAD, './g review' pushes nothing.
# Set G_REVIEW_AUTH to run both pushes inside one authentication command; its
# '{}' is replaced by the command doing the pushes, e.g.
#   export G_REVIEW_AUTH='ssh-agent bash -c "ssh-add ~/.ssh/id_ed25519 && {}"'
# asks for the key passphrase once per './g review' that pushes, and not at
# all when nothing needs pushing.
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

# Whether Gerrit has the given commit as a patch set of a change visible to
# anonymous users, on any branch: Gerrit rejects a push of such a commit as
# "no new changes" whatever the target branch.  Asks the REST API
# anonymously, so no credentials are needed; any failure answers "no".  A
# commit hash covers all its ancestors, so the commits below a known one are
# known too.
on_gerrit() {
    local sha
    sha=$(git rev-parse "$1")
    curl -sf --max-time 30 "https://gerrit.collaboraoffice.com/changes/?q=commit:$sha+project:online" | grep -q '"_number":'
}

# Push everything except the topmost commit as WIP, and the topmost commit
# as the actual review.  Uses GERRIT_REMOTE, TRACKED_BRANCH, TOPIC and
# PUSH_WIP, exported by the review handling below.
push_for_review() {
    if [ -n "$PUSH_WIP" ]; then
        git push $GERRIT_REMOTE HEAD~1:refs/for/$TRACKED_BRANCH%wip${TOPIC:+,topic=$TOPIC}
    fi
    git push $GERRIT_REMOTE HEAD:refs/for/$TRACKED_BRANCH${TOPIC:+%topic=$TOPIC}
}

# Internal: the command that G_REVIEW_AUTH runs.
if [ "$1" == "__review-push" ]; then
    push_for_review
    exit 0
fi

if [ "$1" == "review" ]; then
    # Check before anything is pushed or rebased.
    if [ -n "$G_REVIEW_AUTH" ] && [[ "$G_REVIEW_AUTH" != *'{}'* ]]; then
        echo "Error: G_REVIEW_AUTH has no '{}' for the push command."
        exit 1
    fi

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

    # Options: an optional target branch and an optional Gerrit topic.
    TARGET_BRANCH=
    TOPIC=
    shift
    while [ $# -gt 0 ]; do
        case "$1" in
            --topic=*)
                TOPIC="${1#--topic=}"
                if [ -z "$TOPIC" ]; then
                    echo "Error: '--topic=' needs a topic name."
                    exit 1
                fi
                ;;
            --topic|-t)
                if [ $# -lt 2 ]; then
                    echo "Error: '$1' needs a topic name."
                    exit 1
                fi
                shift
                TOPIC="$1"
                ;;
            -*)
                echo "Error: unknown option '$1'."
                echo "Usage: ./g review [branch] [--topic=NAME]"
                exit 1
                ;;
            *)
                if [ -n "$TARGET_BRANCH" ]; then
                    echo "Error: unexpected argument '$1'."
                    echo "Usage: ./g review [branch] [--topic=NAME]"
                    exit 1
                fi
                TARGET_BRANCH="$1"
                ;;
        esac
        shift
    done

    # Gerrit reads the topic from the push refspec, where '%' starts the
    # options and ',' separates them, so those characters cannot be part of
    # the name.  Check this before anything is pushed.
    if [ -n "$TOPIC" ] && printf '%s' "$TOPIC" | grep -q '[%,[:space:]]'; then
        echo "Error: the topic '$TOPIC' must not contain '%', ',' or whitespace."
        exit 1
    fi

    # If a target branch was passed, set the upstream so the rest of the
    # script can derive REMOTE/TRACKED_BRANCH from it normally.
    if [ -n "$TARGET_BRANCH" ]; then
        git branch --set-upstream-to=$GERRIT_REMOTE/$TARGET_BRANCH $BRANCH
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

    NUM_COMMITS=$(git rev-list --count $REMOTE/$TRACKED_BRANCH..HEAD)
    if [ "$NUM_COMMITS" -lt 1 ]; then
        echo "Error: no commits to push for review."
        exit 1
    fi

    # Only main has a tinderbox job wired up here.
    if [ "$TRACKED_BRANCH" == "main" ]; then
        recommend_known_good_base
    fi

    if on_gerrit HEAD; then
        echo "Note: Gerrit already has HEAD - nothing to push."
        if [ -n "$TOPIC" ]; then
            echo "The topic '$TOPIC' was not applied: Gerrit can't set a topic for unmodified changes."
        fi
        exit 0
    fi
    # The check may have rebased the branch, so count the commits after it.
    PUSH_WIP=
    if [ "$(git rev-list --count $REMOTE/$TRACKED_BRANCH..HEAD)" -gt 1 ] && ! on_gerrit HEAD~1; then
        PUSH_WIP=1
    fi

    export GERRIT_REMOTE TRACKED_BRANCH TOPIC PUSH_WIP
    if [ -z "$G_REVIEW_AUTH" ]; then
        push_for_review
    else
        # An absolute path, as './g' may be run as 'bash g'.  printf %q
        # keeps a space through the quoting of G_REVIEW_AUTH; '$', '\', '"'
        # or a backtick in the path may not survive it.
        self="$(cd "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/$(basename -- "${BASH_SOURCE[0]}")"
        push_command="$(printf %q "$self") __review-push"
        eval "${G_REVIEW_AUTH//'{}'/"$push_command"}"
    fi

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
