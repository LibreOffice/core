#!/usr/bin/env bash
# Render the whole words track with one XRay.Office CLI, reproducibly and in parallel.
#
#   render-words.sh <cli> <outdir> [workers]
#
# SOURCE_DATE_EPOCH and TZ are set because two runs are diffed byte for byte to measure
# reach: without them the PDF's /CreationDate alone moves every file. Ids follow
# batch-check.sh's `<stem>__<ext>` convention so the banked references at
# /c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/ line up name for name.
#
# Adapted from probes/slides-sym-01/render-slides.sh.
set -uo pipefail

CLI="${1:?usage: render-words.sh <cli> <outdir> [workers]}"
OUT="${2:?outdir}"
WORKERS="${3:-4}"
ROOT=/c/sandbox/workdir/sample-files/words

export SOURCE_DATE_EPOCH=1700000000 TZ=UTC
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"

mapfile -t FILES < <(find "$ROOT" -type f \
  \( -iname '*.doc' -o -iname '*.docx' -o -iname '*.rtf' -o -iname '*.odt' -o -iname '*.ott' \) | sort)
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
    timeout 300 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$OUT/$id.pdf"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
echo "rendered $(find "$OUT" -maxdepth 1 -name '*.pdf' | wc -l) of ${#FILES[@]}"
