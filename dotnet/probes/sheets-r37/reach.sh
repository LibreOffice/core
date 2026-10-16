#!/usr/bin/env bash
# reach.sh <track-glob> <base-ours-dir> <newcli> <outdir> <workers>
set -uo pipefail
GLOB="$1"; BASE="$2"; CLI="$3"; OUT="$4"; W="${5:-2}"
export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT/new"
: > "$OUT/reach.tsv"
mapfile -t FILES < <(find /workspace/sample-files/$GLOB -type f | sort)
one() {
  local idx="$1" i=-1 f base ext stem id
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i+1)); [ $((i % W)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"; id="${stem}__${ext,,}"
    rm -rf "$OUT/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 240 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    if [ -f "$OUT/t$idx/$stem.pdf" ]; then mv -f "$OUT/t$idx/$stem.pdf" "$OUT/new/$id.pdf"; fi
    if [ ! -f "$OUT/new/$id.pdf" ]; then echo -e "$id\tnew-failed" >> "$OUT/reach.tsv"; continue; fi
    if [ ! -f "$BASE/$id.pdf" ]; then echo -e "$id\tno-base" >> "$OUT/reach.tsv"; continue; fi
    if cmp -s "$OUT/new/$id.pdf" "$BASE/$id.pdf"; then echo -e "$id\tsame" >> "$OUT/reach.tsv"
    else echo -e "$id\tCHANGED" >> "$OUT/reach.tsv"; fi
  done
}
for ((k=0;k<W;k++)); do one "$k" & done; wait
echo "rows $(wc -l < "$OUT/reach.tsv")"
awk -F'\t' '{c[$2]++} END{for(x in c) print c[x], x}' "$OUT/reach.tsv"
