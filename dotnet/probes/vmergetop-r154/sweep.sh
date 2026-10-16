#!/bin/sh
# Confinement sweep for round 154: render OUR half of the whole 947-document corpus, of the 337
# converted `.odt` and of the 337 converted `.rtf`, once per leg, and diff the legs.
#
# Our half alone, because the diff under test is confined to `dotnet/src` and cannot reach
# `soffice`. `SOURCE_DATE_EPOCH` is pinned or a document printing a date moves on its own. One
# output directory per DOCUMENT, keyed on the whole path -- never per worker slot, and never on the
# basename, because two corpus documents can share one.
set -e
cd "$(dirname "$0")"
OUT=/home/user/vmergetop-r154-sweep
LEG="${1:?usage: sweep.sh <leg>}"
CLI="$PWD/../../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
CORPUS=/home/user/sample-files
mkdir -p "$OUT/$LEG"
stat -c 'CLI mtime %y' "$CLI"

{
    awk -F'\t' 'NR>1 && $3 != "" {print "'"$CORPUS"'/" $3}' "$CORPUS/MANIFEST.tsv"
    ls /home/user/corpus-odf/odt/*.odt
    ls /home/user/corpus-odf/rtf/*.rtf
} | xargs -P 6 -I{} sh -c '
    doc=$(printf %s "{}" | md5sum | cut -c1-12)
    d="'"$OUT/$LEG"'/$doc"
    mkdir -p "$d"
    SOURCE_DATE_EPOCH=0 timeout -k 30 600 "'"$CLI"'" render "{}" --outdir "$d" >/dev/null 2>&1 \
        || echo "FAILED $doc {}"
'
echo "SWEEP-$LEG-COMPLETE $(find "$OUT/$LEG" -name '*.pdf' | wc -l) pdfs"
