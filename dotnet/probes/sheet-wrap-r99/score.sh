#!/bin/bash
set -u
LEG="$1"
echo -e "identity\tsum|ink|%\tpages\tMAJOR"
for f in /home/user/r99-work/mv-$LEG/*.pdf; do
  id=$(basename "$f" .pdf)
  case "$id" in *__ods) R=/home/user/gate-odf-r80/ref/$id.pdf;; *) R=/home/user/gate-orig-r83/ref/$id.pdf;; esac
  [ -f "$R" ] || { echo -e "$id\tno-ref"; continue; }
  python3 /home/user/wt-sheetwrap/.claude/skills/render-comparison/scripts/pdf-image-diff.py "$f" "$R" 2>/dev/null \
   | awk -F'\t' -v id="$id" 'NF>=6 && $1+0>0 {n++; s+=$4; if($6~/MAJOR/) m++} END{printf "%s\t%.2f\t%d\t%d\n", id, s, n, m}'
done
