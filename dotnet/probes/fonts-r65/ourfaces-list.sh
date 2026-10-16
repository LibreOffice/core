#!/usr/bin/env bash
#   ourfaces-list.sh <docs.txt> <out.tsv>
set -uo pipefail
LIST="${1:?}"; OUT="${2:?}"
ROOT=/home/user/sample-files
CLI="${XRAY_OFFICE_CLI:?}"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1700000000}"
: > "$OUT"
while IFS= read -r rel; do
  [ -n "$rel" ] || continue
  d="$WORK/$(printf '%s' "$rel" | md5sum | cut -c1-12)"; mkdir -p "$d"
  timeout 300 "$CLI" render "$ROOT/$rel" --format pdf --outdir "$d" >/dev/null 2>&1
  pdf="$(find "$d" -name '*.pdf' | head -1)"
  if [ -n "$pdf" ]; then
    faces="$(pdffonts "$pdf" 2>/dev/null | tail -n +3 | awk 'NF{print $1}' | sed -E 's/^[A-Z]{6}\+//' | sort -u | paste -sd, -)"
  else faces="(no render)"; fi
  printf '%s\t%s\n' "$rel" "$faces" >> "$OUT"
  rm -rf "$d"
done < "$LIST"
