#!/usr/bin/env bash
# Render our half of the 51-document `.ppt` column and record, per document:
# pages, alphanumeric characters, and the md5 of the PDF. The reference half is the banked
# `/home/user/gate-orig-r83/ref`, which this does not touch -- so two legs of this script are
# directly comparable and the md5 column says which renderings a change reached at all.
set -uo pipefail
CLI="$1"; LIST="$2"; OUT="$3"; KEEP="${4:-}"
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
T=$(mktemp -d -p /home/user/r107-work)
while read -r f; do
  [ -n "$f" ] || continue
  base="$(basename "$f")"; stem="${base%.*}"
  rm -rf "$T/w"; mkdir -p "$T/w"
  timeout -k 30 300 "$CLI" render "/home/user/sample-files/$f" --format pdf --outdir "$T/w" >/dev/null 2>&1
  o="$T/w/$stem.pdf"
  if [ -f "$o" ]; then
    op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
    og=$(pdftotext "$o" - 2>/dev/null | tr -cd '[:alnum:]' | wc -c)
    md5=$(md5sum "$o" | cut -c1-32)
    echo -e "$stem\t$op\t$og\t$md5" >> "$OUT"
    [ -n "$KEEP" ] && { mkdir -p "$KEEP"; cp "$o" "$KEEP/"; }
  else
    echo -e "$stem\tFAILED\t\t" >> "$OUT"
  fi
  rm -rf "$T/w"
done < "$LIST"
rm -rf "$T"
