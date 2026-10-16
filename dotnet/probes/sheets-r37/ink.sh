#!/usr/bin/env bash
# ink.sh <ours-dir> <ref-dir> <ids-file> <out.tsv>
set -uo pipefail
S=/home/user/libreoffice-core/.claude/worktrees/sheets-r37/.claude/skills/render-comparison/scripts
O="$1"; R="$2"; IDS="$3"; T="$4"; : > "$T"
while read -r id; do
  [ -f "$O/$id.pdf" ] && [ -f "$R/$id.pdf" ] || { echo -e "$id\tmissing\t-" >> "$T"; continue; }
  out=$(timeout 600 "$S/pdf-image-diff.py" "$O/$id.pdf" "$R/$id.pdf" --outdir /tmp/inkcmp-$$ 2>/dev/null)
  rc=$?
  if [ $rc -ge 2 ]; then echo -e "$id\trefused\t-" >> "$T"; continue; fi
  echo "$out" | awk -F'\t' -v id="$id" 'NF>=6 && $1 ~ /^[0-9]+$/ {s+=$4; n++} END{printf "%s\tok\t%.2f\t%d\n", id, s, n}' >> "$T"
  rm -rf /tmp/inkcmp-$$
done < "$IDS"
awk -F'\t' '$2=="ok"{s+=$3; n++} END{printf "documents %d  total |ink|%% %.2f\n", n, s}' "$T"
