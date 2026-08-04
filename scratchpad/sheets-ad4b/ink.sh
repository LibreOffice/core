#!/usr/bin/env bash
# The fourth check: over every document whose page count and word count already agree,
# compare the two renderings as images and record the MAJOR pages.
set -u
D=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b
S=/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py
RUN="${1:-dev}"
OUT="$D/ink-$RUN"
mkdir -p "$OUT"
: > "$OUT/summary.tsv"

while IFS=$'\t' read -r path ext pages words fonts _ verdict; do
  [ "$verdict" = "match" ] || continue
  base="$(basename "$path")"; stem="${base%.*}"
  id="${stem}__${ext,,}"
  o="$D/$RUN/ours/$id.pdf"; r="$D/$RUN/ref/$id.pdf"
  [ -f "$o" ] && [ -f "$r" ] || continue
  rm -rf "$OUT/cmp"
  rep="$(timeout 900 python3 "$S" "$o" "$r" --outdir "$OUT/cmp" --quiet 2>&1)"
  echo "$rep" > "$OUT/$id.txt"
  major=$(printf '%s\n' "$rep" | grep -c 'MAJOR')
  total=$(printf '%s\n' "$pages" | cut -d/ -f1)
  printf '%s\t%s\t%s\t%s\n' "$id" "$total" "$major" \
    "$(printf '%s\n' "$rep" | grep -m1 -oP '(?<=  ).*(of page.*)?$' | head -c 120)" \
    >> "$OUT/summary.tsv"
done < "$D/$RUN/rows.tsv"

rm -rf "$OUT/cmp"
awk -F'\t' '{n++; if($3>0){d++; p+=$3}} END{print "documents compared", n, "with MAJOR pages", d, "total MAJOR pages", p}' "$OUT/summary.tsv"
