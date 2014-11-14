#! /bin/bash
#
# check that master contains all the patches from a branch
# and list those that are missing
#

BRANCH="$1"
[ -z "$BRANCH" ] && {
    cat 1>&2 << EOF
check-merged.sh branchname

Checks that all the patches from branch 'branchname' are in master, and
reports the commits that are not.

The check is based on the Change-Id's, so if some commits are missing it, they
won't be detected as missing.
EOF
    exit 1;
}

function collect_change_ids {
    git log `git merge-base origin/master origin/feature/opengl-vcl`.."$1" | \
        sed 's/^commit /XXXcommitXXX/g' | \
        tr '\n' ';' | \
        sed 's/XXXcommitXXX/\n/g' | \
        sed -e 's/;.*Change-Id://' -e 's/;.*$//' | \
        grep -v '^$'> "$2"
}

collect_change_ids "$BRANCH" /tmp/check-merged.branch
collect_change_ids "origin/master" /tmp/check-merged.master

cat /tmp/check-merged.branch | \
    while read COMMIT CHID ; do
        [ -n "$CHID" -a "$CHID" != " " ] && grep -q "$CHID" /tmp/check-merged.master || echo "$COMMIT not in origin/master"
    done
