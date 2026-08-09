#!/usr/bin/env bash
set -uo pipefail
A="$1"; B="$2"
same=0; diff=0; only=0
: > "$B/changed.txt"
for f in "$A"/ours/*.pdf; do
  b="$B/ours/$(basename "$f")"
  if [ ! -f "$b" ]; then only=$((only+1)); echo "MISSING $(basename "$f")" >> "$B/changed.txt"; continue; fi
  if cmp -s "$f" "$b"; then same=$((same+1)); else diff=$((diff+1)); basename "$f" >> "$B/changed.txt"; fi
done
echo "byte-identical $same   changed $diff   missing $only"
