#!/usr/bin/env bash
# Render the 26.2.4.2 reference half for a list of corpus paths. Touches no X-Ray.Office binary,
# so it is safe to run beside a build.
set -uo pipefail
ROOT=/home/user/sample-files
LIST=/home/user/tmp-words67/words-paths.txt
OUT=/home/user/tmp-words67/ref26
SOF=/opt/libreoffice26.2/program/soffice
W=${1:-6}
mapfile -t FILES < "$LIST"
one() {
  local idx=$1 i=-1 f base ext stem id
  local prof=/home/user/tmp-words67/prof/p$idx t=/home/user/tmp-words67/prof/t$idx
  mkdir -p "$prof"
  for f in "${FILES[@]}"; do
    i=$((i+1)); [ $((i % W)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    [ -s "$OUT/$id.pdf" ] && continue
    rm -rf "$t"; mkdir -p "$t"
    timeout 420 "$SOF" -env:UserInstallation="file://$prof" --headless \
      --convert-to pdf --outdir "$t" "$ROOT/$f" >/dev/null 2>&1
    [ -f "$t/$stem.pdf" ] && mv -f "$t/$stem.pdf" "$OUT/$id.pdf"
  done
  rm -rf "$t"
}
export W
for k in $(seq 0 $((W-1))); do one "$k" & done
wait
echo "DONE ref26: $(ls "$OUT" | wc -l) of ${#FILES[@]}"
