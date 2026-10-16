#!/usr/bin/env bash
set -uo pipefail
CLI="$1"; OUT="$2"; LIST="$3"
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
T=$(mktemp -d)
while read -r f; do
  [ -n "$f" ] || continue
  base="$(basename "$f")"; stem="${base%.*}"
  rm -rf "$T/w"; mkdir -p "$T/w"
  timeout -k 30 300 "$CLI" render "/home/user/sample-files/$f" --format pdf --outdir "$T/w" >/dev/null 2>&1
  if [ -f "$T/w/$stem.pdf" ]; then echo -e "$f\t$(md5sum "$T/w/$stem.pdf" | cut -d' ' -f1)" >> "$OUT"
  else echo -e "$f\tFAILED" >> "$OUT"; fi
  rm -rf "$T/w"
done < "$LIST"
rm -rf "$T"
