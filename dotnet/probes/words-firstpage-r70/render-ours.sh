#!/usr/bin/env bash
# Render the words track with one XRay.Office CLI into one directory.
#   render-ours.sh <cli> <outdir> <path-list> [workers]
# `words-ink-r67/render-ours.sh` with its list and root as arguments rather than baked in.
set -uo pipefail
CLI="$1"; OUT="$2"; LIST="$3"; W="${4:-6}"
ROOT=/home/user/sample-files
export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT"
mapfile -t FILES < "$LIST"
one() {
  local idx=$1 i=-1 f base ext stem id t
  t="$OUT/.t$idx"
  for f in "${FILES[@]}"; do
    i=$((i+1)); [ $((i % W)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"; id="${stem}__${ext,,}"
    [ -s "$OUT/$id.pdf" ] && continue
    rm -rf "$t"; mkdir -p "$t"
    timeout 400 "$CLI" render "$ROOT/$f" --format pdf --outdir "$t" >/dev/null 2>&1
    [ -f "$t/$stem.pdf" ] && mv -f "$t/$stem.pdf" "$OUT/$id.pdf"
  done
  rm -rf "$t"
}
export W OUT CLI ROOT
for k in $(seq 0 $((W-1))); do one "$k" & done
wait
echo "DONE $OUT: $(ls "$OUT" | wc -l) of ${#FILES[@]}"
