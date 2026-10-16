#!/bin/sh
# Confinement sweep for round 149: render OUR half of the whole 947-document corpus and of the 337
# converted `.odt` twice -- once at the round's base and once with the fix -- and diff.
#
# `SOURCE_DATE_EPOCH` is pinned, because a document printing a date draws different ink on a
# different second and an unpinned sweep reports EVERYTHING moving (dotnet/CLAUDE.md records a round
# that read 176 of 176 that way). One output directory per DOCUMENT, never per worker slot.
# The document list is MANIFEST.tsv's own paths, never a `find` total: the case-insensitive mount
# materialises alias entries when a tool resolves a document, so a `find` count drifts upwards on
# its own.
set -e
cd "$(dirname "$0")"
OUT=/home/user/rowband-r149-sweep
LEG="${1:?usage: sweep.sh <leg>}"
CLI="$PWD/../../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
CORPUS=/home/user/sample-files
mkdir -p "$OUT/$LEG"

{
    awk -F'\t' 'NR>1 && $3 != "" {print "'"$CORPUS"'/" $3}' "$CORPUS/MANIFEST.tsv"
    ls /home/user/corpus-odf/odt/*.odt
} | xargs -P 6 -I{} sh -c '
    # Keyed on the whole path, not the basename: two corpus documents can share a basename and
    # would then land in one directory, where each render deletes what the other wrote.
    doc=$(printf %s "{}" | md5sum | cut -c1-12)
    d="'"$OUT/$LEG"'/$doc"
    mkdir -p "$d"
    SOURCE_DATE_EPOCH=0 timeout -k 30 600 "'"$CLI"'" render "{}" --outdir "$d" >/dev/null 2>&1 \
        || echo "FAILED $doc"
'
find "$OUT/$LEG" -name '*.pdf' | wc -l
