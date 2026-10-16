#!/bin/sh
# Render EVERY corpus document with a given XRay.Office.Cli into <out>, one directory per
# document -- never per worker slot, which is how probes/chart-layout lost 124 renders.
# Reference-free: nothing here calls soffice.
#   sweep-ours.sh <corpus> <cli> <out> <jobs>
set -eu
CORPUS=$1; CLI=$2; OUT=$3; JOBS=${4:-3}
mkdir -p "$OUT"
export SOURCE_DATE_EPOCH=1700000000
awk -F'\t' 'NR>1 {print $3}' "$CORPUS/MANIFEST.tsv" |
while IFS= read -r rel; do
  base=$(basename "$rel"); stem=${base%.*}; ext=${base##*.}
  printf '%s__%s\t%s\n' "$stem" "$ext" "$rel"
done > "$OUT/list.tsv"

render_one() {
  id=$1; rel=$2
  d="$OUT/w/$id"; rm -rf "$d"; mkdir -p "$d"
  if timeout -k 30 600 "$CLI" render "$CORPUS/$rel" --format pdf --outdir "$d" >"$d/log" 2>&1; then
    p=$(ls "$d"/*.pdf 2>/dev/null | head -1)
    if [ -n "$p" ]; then mv "$p" "$OUT/$id.pdf"; else echo "ours-failed	$id" >> "$OUT/fail.txt"; fi
  else
    echo "ours-failed	$id" >> "$OUT/fail.txt"
  fi
  rm -rf "$d"
}

i=0
while IFS="$(printf '\t')" read -r id rel; do
  render_one "$id" "$rel" &
  i=$((i+1))
  if [ $((i % JOBS)) -eq 0 ]; then wait; fi
done < "$OUT/list.tsv"
wait
ls "$OUT"/*.pdf | wc -l
