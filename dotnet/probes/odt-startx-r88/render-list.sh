#!/usr/bin/env bash
# Render every document in a list file into <outdir>, keyed by stem and extension.
#   render-list.sh <list> <outdir> [workers]
set -uo pipefail
LIST="${1:?list file}"; OUT="${2:?outdir}"; WORKERS="${3:-3}"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}"
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
mapfile -t FILES < "$LIST"
one() {
  local idx="$1" i=-1 f base ext stem
  for f in "${FILES[@]}"; do
    i=$((i+1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout -k 30 240 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$OUT/${stem}__${ext,,}.pdf"
  done
  rm -rf "${OUT:?}/t$idx"
}
for w in $(seq 0 $((WORKERS-1))); do one "$w" & done
wait
ls "$OUT"/*.pdf | wc -l
