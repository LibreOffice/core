#!/usr/bin/env bash
# Our own face set for every document in a directory, one row per file.
#   ourfaces-dir.sh <srcdir> <out.tsv>
set -uo pipefail
SRC="${1:?usage: ourfaces-dir.sh <srcdir> <out.tsv>}"
OUT="${2:?usage: ourfaces-dir.sh <srcdir> <out.tsv>}"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}"
WORK="$(mktemp -d "$(dirname "$OUT")/ours.XXXXXX")"
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1700000000}"
: > "$OUT"
for f in "$SRC"/*.docx; do
  stem="$(basename "$f" .docx)"
  d="$WORK/$stem"; mkdir -p "$d"
  timeout 300 "$CLI" render "$f" --format pdf --outdir "$d" >/dev/null 2>&1
  pdf="$(find "$d" -name '*.pdf' | head -1)"
  if [ -n "$pdf" ]; then
    faces="$(pdffonts "$pdf" 2>/dev/null | tail -n +3 | awk 'NF{print $1}' \
             | sed -E 's/^[A-Z]{6}\+//' | sort -u | paste -sd, -)"
  else
    faces="(no render)"
  fi
  printf '%s\t%s\n' "$stem" "$faces" >> "$OUT"
done
rm -rf "$WORK"
cat "$OUT"
