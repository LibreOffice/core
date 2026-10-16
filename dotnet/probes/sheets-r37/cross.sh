#!/usr/bin/env bash
# cross.sh <glob> <cliA> <cliB> <outdir> <workers>
set -uo pipefail
GLOB="$1"; A="$2"; B="$3"; OUT="$4"; W="${5:-2}"
export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT"; : > "$OUT/cross.tsv"
mapfile -t FILES < <(find /workspace/sample-files/$GLOB -type f | sort)
one() {
  local idx="$1" i=-1 f base stem id
  for f in "${FILES[@]}"; do
    i=$((i+1)); [ $((i % W)) -eq "$idx" ] || continue
    base="$(basename "$f")"; stem="${base%.*}"; id="${stem}__${base##*.}"
    rm -rf "$OUT/a$idx" "$OUT/b$idx"; mkdir -p "$OUT/a$idx" "$OUT/b$idx"
    timeout 240 "$A" render "$f" --format pdf --outdir "$OUT/a$idx" >/dev/null 2>&1
    timeout 240 "$B" render "$f" --format pdf --outdir "$OUT/b$idx" >/dev/null 2>&1
    if [ -f "$OUT/a$idx/$stem.pdf" ] && [ -f "$OUT/b$idx/$stem.pdf" ]; then
      if cmp -s "$OUT/a$idx/$stem.pdf" "$OUT/b$idx/$stem.pdf"; then echo -e "$id\tsame" >> "$OUT/cross.tsv"
      else echo -e "$id\tCHANGED" >> "$OUT/cross.tsv"; fi
    else echo -e "$id\tfailed" >> "$OUT/cross.tsv"; fi
  done
  rm -rf "$OUT/a$idx" "$OUT/b$idx"
}
for ((k=0;k<W;k++)); do one "$k" & done; wait
echo "rows $(wc -l < "$OUT/cross.tsv")"
awk -F'\t' '{c[$2]++} END{for(x in c) print c[x], x}' "$OUT/cross.tsv"
