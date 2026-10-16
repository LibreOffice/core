#!/bin/sh
# Confinement sweep for round 148: render OUR half of the 337 converted `.odt` twice -- once at the
# round's base and once with the fix -- and diff. `SOURCE_DATE_EPOCH` is pinned because a document
# printing a date draws different ink on a different second, and one output directory per DOCUMENT
# (never per worker slot) because a thread pool does not work consecutive indices.
set -e
cd "$(dirname "$0")/../.."
OUT=/home/user/odtscale-r148-sweep
LEG=$1   # `base` or `head`
mkdir -p "$OUT/$LEG"
CLI=tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli

ls /home/user/corpus-odf/odt/*.odt | xargs -P 6 -I{} sh -c '
    doc=$(basename "{}" .odt)
    d="'"$OUT/$LEG"'/$doc"
    mkdir -p "$d"
    SOURCE_DATE_EPOCH=0 timeout 300 '"$PWD/$CLI"' render "{}" --outdir "$d" >/dev/null 2>&1 \
        || echo "FAILED $doc"
'
find "$OUT/$LEG" -name '*.pdf' | wc -l
