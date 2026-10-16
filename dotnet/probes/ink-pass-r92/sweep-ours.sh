#!/bin/sh
# Render one track's documents with a given XRay.Office.Cli into <out>, one directory per
# document (never per worker slot -- two live renders in one directory destroy each other).
# Reference-free: the banked reference is reused.
#   sweep-ours.sh <corpus> <family> <cli> <out> <jobs>
set -eu
CORPUS=$1; FAMILY=$2; CLI=$3; OUT=$4; JOBS=${5:-3}
mkdir -p "$OUT"
export SOURCE_DATE_EPOCH=1700000000
awk -F'\t' -v f="$FAMILY" 'NR>1 && $1==f {print $3}' "$CORPUS/MANIFEST.tsv" |
while IFS= read -r rel; do
  base=$(basename "$rel"); stem=${base%.*}; ext=${base##*.}
  id="${stem}__${ext}"
  printf '%s\t%s\n' "$id" "$rel"
done > "$OUT/list.tsv"

render_one() {
  id=$1; rel=$2
  d="$OUT/w/$id"; rm -rf "$d"; mkdir -p "$d"
  if timeout -k 30 300 "$CLI" render "$CORPUS/$rel" --format pdf --outdir "$d" >"$d/log" 2>&1; then
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
