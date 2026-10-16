#!/usr/bin/env bash
# Render each .ppt and reduce it to its content stream's own (page, font, size, x, baseline)
# table with tfz.py, deleting the rendering as it goes.  The reference half is read from the bank.
set -uo pipefail
CLI="$1"; OUT="$2"
export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT"
T=$(mktemp -d -p /home/user/r100-work)
while read -r f; do
  [ -n "$f" ] || continue
  stem="$(basename "$f")"; stem="${stem%.*}"
  rm -rf "$T/w"; mkdir -p "$T/w"
  timeout -k 30 300 "$CLI" render "/home/user/sample-files/$f" --format pdf --outdir "$T/w" >/dev/null 2>&1
  if [ -f "$T/w/$stem.pdf" ]; then
    python3 /home/user/r100-work/tfz.py "$T/w/$stem.pdf" 2>/dev/null | cut -f2- > "$OUT/$stem.ours.tsv"
  else
    echo FAILED > "$OUT/$stem.ours.tsv"
  fi
  r="/home/user/gate-orig-r83/ref/${stem}__ppt.pdf"
  [ -f "$r" ] && python3 /home/user/r100-work/tfz.py "$r" 2>/dev/null | cut -f2- > "$OUT/$stem.ref.tsv"
  rm -rf "$T/w"
done < /home/user/r100-work/ppt.list
rm -rf "$T"
