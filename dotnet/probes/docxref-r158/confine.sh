#!/bin/sh
# Renders every corpus DOCX with the binary under test and compares it byte for byte against the
# bank this round started from.  SOURCE_DATE_EPOCH is pinned so a document printing today's date
# does not read as a mover.
set -u
CLI=${XRAY_OFFICE_CLI:?}
BANK=${BANK:-/home/user/ww8covered-r156-sweep/head}
OUT=${OUT:?}
mkdir -p "$OUT/renders"
: > "$OUT/rows.tsv"
find /home/user/sample-files -iname '*.docx' -o -iname '*.docm' | sort | while read -r src; do
    stem=$(basename "$src"); stem=${stem%.*}
    key=$(printf '%s' "$src" | md5sum | cut -c1-12)
    d="$OUT/renders/$key"; mkdir -p "$d"
    SOURCE_DATE_EPOCH=0 timeout -k 30 300 "$CLI" render "$src" --outdir "$d" >/dev/null 2>&1
    new=$(find "$d" -name '*.pdf' | head -1)
    old=$(find "$BANK" -name "$stem.pdf" | head -1)
    if [ -z "$new" ]; then verdict=ours-failed
    elif [ -z "$old" ]; then verdict=no-bank
    elif cmp -s "$old" "$new"; then verdict=same
    else verdict=MOVED
    fi
    printf '%s\t%s\n' "$verdict" "$stem" >> "$OUT/rows.tsv"
done
