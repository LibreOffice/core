#!/usr/bin/env bash
set -u
D=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b
S=/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py
RUN=dev
OUT="$D/ink-$RUN"
while IFS=$'\t' read -r path ext pages words fonts _ verdict; do
  [ "$verdict" = "match" ] || continue
  base="$(basename "$path")"; stem="${base%.*}"
  id="${stem}__${ext,,}"
  grep -qF "$id	" "$OUT/summary.tsv" && continue
  o="$D/$RUN/ours/$id.pdf"; r="$D/$RUN/ref/$id.pdf"
  [ -f "$o" ] && [ -f "$r" ] || { echo "missing pdf: $id"; continue; }
  rm -rf "$OUT/cmp"
  rep="$(timeout 1200 python3 "$S" "$o" "$r" --outdir "$OUT/cmp" --quiet 2>&1)"
  echo "$rep" > "$OUT/$id.txt"
  major=$(printf '%s\n' "$rep" | grep -c 'MAJOR')
  printf '%s\t%s\t%s\t\n' "$id" "$(printf '%s' "$pages" | cut -d/ -f1)" "$major" >> "$OUT/summary.tsv"
  echo "did $id: $major major"
done < "$D/$RUN/rows.tsv"
rm -rf "$OUT/cmp"
