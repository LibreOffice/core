#!/usr/bin/env bash
# render-slides.sh <cli> <outdir> [workers]
set -uo pipefail
CLI="$1"; OUT="$2"; W="${3:-4}"
mkdir -p "$OUT"; OUT="$(cd "$OUT" && pwd)"
export SOURCE_DATE_EPOCH=1700000000
mapfile -t FILES < <(find /c/sandbox/workdir/sample-files/slides -type f \
  \( -iname '*.ppt' -o -iname '*.pptx' -o -iname '*.odp' -o -iname '*.otp' \) | sort)
echo "${#FILES[@]} files" >&2
one() {
  local idx="$1" i=-1 f base ext stem id
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i+1)); [ $((i % W)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 300 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    if [ -f "$OUT/t$idx/$stem.pdf" ]; then mv -f "$OUT/t$idx/$stem.pdf" "$OUT/$id.pdf"; fi
  done
  rm -rf "${OUT:?}/t$idx"
}
for ((k=0;k<W;k++)); do one "$k" & done
wait
ls "$OUT"/*.pdf | wc -l
