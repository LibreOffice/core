#!/usr/bin/env bash
# Render every document of the sheets track with two CLIs and compare the bytes.
#
# SOURCE_DATE_EPOCH is pinned so the seventeen documents that print the date in a header
# cannot contribute — round twenty-five measured that floor and it is the whole reason the
# variable is honoured at all.
set -uo pipefail
S=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/sheets-r27
A="${1:?usage: reach.sh <cli-a> <cli-b> <outdir>}"
B="${2:?}"
OUT="${3:?}"
export SOURCE_DATE_EPOCH=1700000000
export TZ=UTC

mkdir -p "$OUT/a" "$OUT/b"
: > "$OUT/reach.tsv"

mapfile -t FILES < <(find /workspace/sample-files/sheets -type f | sort)
echo "${#FILES[@]} documents" >&2

for f in "${FILES[@]}"; do
  rel="${f#/workspace/sample-files/}"
  key=$(echo "$rel" | tr '/ ' '__')
  "$A" render "$f" --format pdf --outdir "$OUT/a/$key" > /dev/null 2>&1
  "$B" render "$f" --format pdf --outdir "$OUT/b/$key" > /dev/null 2>&1
  pa=$(find "$OUT/a/$key" -name '*.pdf' 2>/dev/null | head -1)
  pb=$(find "$OUT/b/$key" -name '*.pdf' 2>/dev/null | head -1)
  if [ -z "$pa" ] || [ -z "$pb" ]; then
    printf '%s\tno-output\n' "$rel" >> "$OUT/reach.tsv"
  elif cmp -s "$pa" "$pb"; then
    printf '%s\tsame\n' "$rel" >> "$OUT/reach.tsv"
  else
    printf '%s\tdiffers\n' "$rel" >> "$OUT/reach.tsv"
  fi
  rm -rf "$OUT/a/$key" "$OUT/b/$key"
done

echo "--- $(wc -l < "$OUT/reach.tsv") rows"
sort "$OUT/reach.tsv" | awk -F'\t' '{c[$2]++} END {for (k in c) print c[k], k}'
grep -P '\tdiffers$' "$OUT/reach.tsv" || true
