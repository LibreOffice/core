#!/bin/sh
# Render round 146's own fixtures with THIS tree, into a leg directory. The reference half is
# already banked at `probes/tablerow-r146/out/` and is not re-rendered: the diff under test is
# confined to `dotnet/src`, which cannot reach `soffice`.
set -e
cd "$(dirname "$0")"
LEG="${1:?usage: render-ours.sh <leg>}"
CLI=../../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
mkdir -p "$LEG"
for f in ../tablerow-r146/fixtures/*.docx; do
    b=$(basename "$f" .docx)
    SOURCE_DATE_EPOCH=0 "$CLI" render "$f" --outdir "$LEG" >/dev/null 2>&1 || echo "FAIL $b"
    [ -s "$LEG/$b.pdf" ] || echo "MISSING $b"
done
ls "$LEG"/*.pdf | wc -l
