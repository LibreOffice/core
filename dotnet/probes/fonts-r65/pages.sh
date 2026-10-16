#!/usr/bin/env bash
# Page counts for a named list of documents, through one binary.
#   pages.sh <docs.txt> <out.tsv>
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
  n="$( [ -n "$pdf" ] && pdfinfo "$pdf" 2>/dev/null | awk '/^Pages/{print $2}' || echo "-")"
  printf '%s\t%s\n' "$rel" "${n:--}" >> "$OUT"
  rm -rf "$d"
done < "$LIST"
