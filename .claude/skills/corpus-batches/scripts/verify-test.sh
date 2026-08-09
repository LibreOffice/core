#!/usr/bin/env bash
#
# Verify that a test actually detects the defect it claims to, by putting the defect back
# and watching the test fail.
#
#   verify-test.sh <project> '<mutation command>' [filter]
#
#   verify-test.sh Paperless.Core \
#       "sed -i 's/IsTitleBold = true/IsTitleBold = false/' src/Paperless.Core/Charts/ChartPlot.cs" \
#       ChartAutoText
#
# Why this exists rather than a paragraph of advice. The cycle deliberately puts the tree in
# a state that looks like a mistake, and every ordinary gesture for cleaning up a mistake is
# then wrong:
#
#   * `git checkout -- <file>` discards the file, not the patch — so it takes any
#     *uncommitted* real work in that file with it. An agent lost a working fix that way,
#     and the next agent lost one again with the warning open in front of it. That is what
#     this script exists to make impossible.
#   * `git add -A` while the patch is applied commits the defect.
#   * `--no-build` afterwards measures the defect.
#   * `mv backup original` restores an *older* mtime, so MSBuild skips the rebuild and even
#     a plain `dotnet build` reports success over a binary that still holds the defect.
#
# So: this refuses to start unless the tree is clean, which means the restore can never
# destroy anything; it rebuilds explicitly on both legs; and it names the tests that failed
# rather than only counting them.
#
# Exit status is the finding, not an error:
#   0  the mutation broke at least one test — the test detects the defect
#   1  the mutation broke nothing — the test is a drift guard, not a detector
#   2  refused to run, or the build/mutation failed
#
# A `1` is a legitimate outcome to report and label. It is not a reason to weaken the test:
# rounds on this project have kept such cases deliberately, documented as drift guards, and
# that honesty is what makes the detectors credible.

set -uo pipefail

ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || {
    echo "verify-test: not in a git repository" >&2; exit 2
}
cd "$ROOT" || exit 2

if [ $# -lt 2 ]; then
    sed -n '2,12p' "$0" | sed 's/^# \?//'
    exit 2
fi

PROJECT="$1"
MUTATION="$2"
FILTER="${3:-}"

CSPROJ="dotnet/tests/${PROJECT}.Tests/${PROJECT}.Tests.csproj"
[ -f "$CSPROJ" ] || CSPROJ="dotnet/tests/${PROJECT}/${PROJECT}.csproj"
if [ ! -f "$CSPROJ" ]; then
    echo "verify-test: no test project for '$PROJECT'" >&2
    exit 2
fi

# The one guard that matters. A clean tree means `git checkout -- .` at the end is
# guaranteed to restore exactly the committed state and cannot destroy work.
if [ -n "$(git status --porcelain -- dotnet)" ]; then
    echo "verify-test: refusing to run — dotnet/ has uncommitted changes." >&2
    echo >&2
    git status --short -- dotnet >&2
    echo >&2
    echo "Commit first. The restore step discards the working tree, so an uncommitted fix" >&2
    echo "in a mutated file would be lost — which has happened twice on this project." >&2
    echo "A wip: commit is enough; squash it later." >&2
    exit 2
fi

BASE="$(git rev-parse --short HEAD)"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

restore() {
    git checkout -- dotnet 2>/dev/null
    # Restoring rewrites the files, so mtimes advance and MSBuild rebuilds. Belt and braces:
    # touch them anyway, because a checkout that changed nothing leaves the mtime alone.
    find dotnet/src dotnet/tests -name '*.cs' -newermt '-1 minute' -exec touch {} + 2>/dev/null
    dotnet build dotnet/Paperless.slnx >"$OUT/restore-build.txt" 2>&1
}

echo "verify-test: $PROJECT at $BASE${FILTER:+, filter $FILTER}"
echo "verify-test: applying mutation"

if ! eval "$MUTATION"; then
    echo "verify-test: the mutation command failed" >&2
    restore
    exit 2
fi

if [ -z "$(git status --porcelain -- dotnet)" ]; then
    echo "verify-test: the mutation changed nothing — check the pattern matches" >&2
    exit 2
fi

echo "verify-test: files touched by the mutation:"
git status --short -- dotnet | sed 's/^/  /'

# Explicit build. Never --no-build here: that is how a clean tree reported failures once.
if ! dotnet build dotnet/Paperless.slnx >"$OUT/build.txt" 2>&1; then
    echo "verify-test: the mutated tree does not build — that is not a detection." >&2
    echo "verify-test: a mutation must be a behaviour change, not a compile error." >&2
    grep -E 'error [A-Z]+[0-9]+' "$OUT/build.txt" | head -5 >&2
    restore
    exit 2
fi

if [ -n "$FILTER" ]; then
    dotnet test "$CSPROJ" --no-build --filter "$FILTER" >"$OUT/test.txt" 2>&1
else
    dotnet test "$CSPROJ" --no-build >"$OUT/test.txt" 2>&1
fi

SUMMARY="$(grep -oE '(Passed!|Failed!)[^\r]*' "$OUT/test.txt" | head -1)"
FAILED="$(grep -oE '^\s*(Failed|X)\s+\S+' "$OUT/test.txt" | sed 's/^[[:space:]]*//' | sort -u)"

echo "verify-test: restoring $BASE"
restore

echo
echo "  $SUMMARY"

if [ -n "$FAILED" ]; then
    echo
    echo "  the mutation was DETECTED by:"
    echo "$FAILED" | sed 's/^/    /'
    exit 0
fi

echo
echo "  the mutation was NOT detected by any test in $PROJECT."
echo "  Either the test is a drift guard rather than a detector — label it as one and say so —"
echo "  or the mutation is an equivalent formulation rather than a defect. Those are different"
echo "  findings and both are worth reporting; do not report one as the other."
exit 1
