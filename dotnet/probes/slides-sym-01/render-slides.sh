#!/usr/bin/env bash
# Render the whole slides track with one X-Ray.Office CLI, reproducibly and in parallel.
#
#   render-slides.sh <cli> <outdir> [workers]
#
# SOURCE_DATE_EPOCH and TZ are set because the output of two runs is diffed byte for byte:
# without them the PDF's /CreationDate alone moves every file. Ids follow batch-check.sh's
# `<stem>__<ext>` convention so the banked references at
# /c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides/ line up name for name.
set -uo pipefail

CLI="${1:?usage: render-slides.sh <cli> <outdir> [workers]}"
OUT="${2:?outdir}"
WORKERS="${3:-4}"
ROOT=/c/sandbox/workdir/sample-files/slides

export SOURCE_DATE_EPOCH=1700000000 TZ=UTC
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"

mapfile -t FILES < <(find "$ROOT" -type f \( -iname '*.ppt' -o -iname '*.pptx' \) | sort)
echo "${#FILES[@]} documents, cli $CLI" >&2

one() {
  local idx="$1" i=-1 f base ext stem id
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    [ -f "$OUT/$id.pdf" ] && continue
    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 240 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$OUT/$id.pdf"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
echo "rendered $(find "$OUT" -maxdepth 1 -name '*.pdf' | wc -l) of ${#FILES[@]}"
