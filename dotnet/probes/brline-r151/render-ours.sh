#!/bin/sh
# Render round 149's own arms with THIS tree, into a leg directory. The reference half is already
# banked as `probes/brline-r149/measured*.txt` and is not re-rendered.
#
# Round 149 could measure nothing on our side because the parent's build was live throughout, and
# it said so rather than reporting unattributable figures. This is that render, with the tree
# quiescent; the binary's mtime is printed so the leg can be attributed to a build.
set -e
cd "$(dirname "$0")"
CLI=../../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
LEG="${1:?usage: render-ours.sh <leg> <fixture-dir>...}"
shift
mkdir -p "$LEG"
stat -c 'binary %y' "$CLI"
for dir in "$@"; do
    for f in "$dir"/*.docx; do
        b=$(basename "$f" .docx)
        SOURCE_DATE_EPOCH=0 "$CLI" render "$f" --outdir "$LEG" >/dev/null 2>&1 || echo "FAIL $b"
        [ -s "$LEG/$b.pdf" ] || echo "MISSING $b"
    done
done
ls "$LEG"/*.pdf | wc -l
