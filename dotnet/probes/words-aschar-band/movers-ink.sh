#!/usr/bin/env bash
# |ink|% against 26.2.4.2 for the documents one change moved, before and after.
set -uo pipefail
D=/home/user/wt-words67/.claude/skills/render-comparison/scripts/pdf-image-diff.py
printf "%-52s %8s %8s %8s\n" document abs_before abs_after delta
while read -r id; do
  b=$(python3 "$D" /home/user/tmp-words67/ours-before/"$id".pdf /home/user/tmp-words67/ref26/"$id".pdf 2>/dev/null \
      | awk -F'\t' '$1 ~ /^[0-9]+$/ && $4 ~ /^[0-9.]+$/ {s+=$4} END{printf "%.2f", s}')
  a=$(python3 "$D" /home/user/tmp-words67/ours-after/"$id".pdf /home/user/tmp-words67/ref26/"$id".pdf 2>/dev/null \
      | awk -F'\t' '$1 ~ /^[0-9]+$/ && $4 ~ /^[0-9.]+$/ {s+=$4} END{printf "%.2f", s}')
  printf "%-52s %8s %8s %8s\n" "$id" "${b:--}" "${a:--}" "$(awk -v x="$b" -v y="$a" 'BEGIN{if(x==""||y=="")print "-";else printf "%+.2f", y-x}')"
done < /home/user/tmp-words67/movers.txt
