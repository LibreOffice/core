#!/bin/sh
# Render whatever `resume.py` says a leg is still missing, with the same settings `sweep.sh` uses.
set -e
cd "$(dirname "$0")"
OUT=/home/user/vmergetop-r154-sweep
LEG="${1:?usage: resume-render.sh <leg>}"
CLI="$PWD/../../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
stat -c 'CLI dll mtime %y' "$(dirname "$CLI")/XRay.Office.WordProcessing.dll"

python3 resume.py "$LEG" --list | xargs -P 6 -I{} sh -c '
    doc=$(printf %s "{}" | md5sum | cut -c1-12)
    d="'"$OUT/$LEG"'/$doc"
    mkdir -p "$d"
    SOURCE_DATE_EPOCH=0 timeout -k 30 600 "'"$CLI"'" render "{}" --outdir "$d" >/dev/null 2>&1 \
        || echo "FAILED $doc {}"
'
echo "RESUME-$LEG-COMPLETE $(find "$OUT/$LEG" -name '*.pdf' | wc -l) pdfs"
