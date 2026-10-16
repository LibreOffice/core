#!/usr/bin/env bash
# render-ours.sh <cli> <outdir> <file-list>
#
# Render every path in the list with one binary, under SOURCE_DATE_EPOCH so that two runs of
# the same binary are byte-equal with nothing masked. Used to show confinement: a track the
# diff cannot reach must come back byte for byte identical at the two binaries.
set -uo pipefail
CLI="${1:?usage: render-ours.sh <cli> <outdir> <file-list>}"
OUT="${2:?outdir}"
LIST="${3:?file list, one absolute path per line}"
WORKERS="${WORKERS:-3}"

mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
export SOURCE_DATE_EPOCH=1757289600

mapfile -t FILES < "$LIST"

one() {
  local idx="$1" i=-1 f base ext stem id
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout -k 30 900 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$OUT/$id.pdf"
  done
  rm -rf "${OUT:?}/t$idx"
}

for ((w = 0; w < WORKERS; w++)); do one "$w" & done
wait
echo "RENDER-DONE $(find "$OUT" -maxdepth 1 -name '*.pdf' | wc -l) of ${#FILES[@]}"
