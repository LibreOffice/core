#!/bin/sh
# Confinement sweep for the O100 fix: render OUR half of the whole corpus and of the two converted
# ODF columns twice and diff. SOURCE_DATE_EPOCH pinned; one output directory per DOCUMENT keyed on
# the whole path, because two corpus documents can share a basename.
set -e
cd "$(dirname "$0")"
OUT=/home/user/odscentre-r150-sweep
LEG="${1:?usage: sweep.sh <leg>}"
CLI="$PWD/../../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
CORPUS=/home/user/sample-files
mkdir -p "$OUT/$LEG"

{
    awk -F'\t' 'NR>1 && $3 != "" {print "'"$CORPUS"'/" $3}' "$CORPUS/MANIFEST.tsv"
    ls /home/user/corpus-odf/ods/*.ods
    ls /home/user/corpus-odf/odt/*.odt
} | xargs -P 6 -I{} sh -c '
    doc=$(printf %s "{}" | md5sum | cut -c1-12)
    d="'"$OUT/$LEG"'/$doc"
    mkdir -p "$d"
    SOURCE_DATE_EPOCH=0 timeout -k 30 600 "'"$CLI"'" render "{}" --outdir "$d" >/dev/null 2>&1 \
        || echo "FAILED $doc"
'
find "$OUT/$LEG" -name '*.pdf' | wc -l
