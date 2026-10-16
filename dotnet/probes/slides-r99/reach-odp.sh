#!/usr/bin/env bash
# Base against head, bytes only -- there is no banked ODF reference in this container.
set -uo pipefail
S=/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/r99
export SOURCE_DATE_EPOCH=1700000000
: > "$S/reach-odp.tsv"
T=$(mktemp -d)
while read -r f; do
  [ -n "$f" ] || continue
  base="$(basename "$f")"; stem="${base%.*}"
  rm -rf "$T/b" "$T/h"; mkdir -p "$T/b" "$T/h"
  timeout -k 30 300 "$S/cli-base/XRay.Office.Cli" render "$f" --format pdf --outdir "$T/b" >/dev/null 2>&1
  timeout -k 30 300 "$S/cli-head/XRay.Office.Cli" render "$f" --format pdf --outdir "$T/h" >/dev/null 2>&1
  if [ ! -f "$T/b/$stem.pdf" ] || [ ! -f "$T/h/$stem.pdf" ]; then echo -e "$base\tFAILED" >> "$S/reach-odp.tsv"
  elif cmp -s "$T/b/$stem.pdf" "$T/h/$stem.pdf"; then echo -e "$base\tidentical" >> "$S/reach-odp.tsv"
  else echo -e "$base\tmoved" >> "$S/reach-odp.tsv"; fi
  rm -rf "$T/b" "$T/h"
done < "$S/odp-sample.txt"
rm -rf "$T"
cut -f2 "$S/reach-odp.tsv" | sort | uniq -c
