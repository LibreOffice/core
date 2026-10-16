#!/usr/bin/env bash
set -uo pipefail
CLI="$1"; OUT="$2"
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
T=$(mktemp -d -p /home/user/r100-work)
while read -r f; do
  [ -n "$f" ] || continue
  stem="$(basename "$f")"; stem="${stem%.*}"
  rm -rf "$T/w"; mkdir -p "$T/w"
  timeout -k 30 300 "$CLI" render "/home/user/sample-files/$f" --format pdf --outdir "$T/w" >/dev/null 2>&1
  [ -f "$T/w/$stem.pdf" ] && python3 /home/user/r100-work/sizes.py "$T/w/$stem.pdf" "${stem}__ppt" >> "$OUT" 2>/dev/null
  rm -rf "$T/w"
done < /home/user/r100-work/ppt.list
rm -rf "$T"
