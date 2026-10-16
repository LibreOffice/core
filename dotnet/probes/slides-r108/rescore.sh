#!/usr/bin/env bash
set -uo pipefail
D=/home/user/wt-slidesize/.claude/skills/render-comparison/scripts/pdf-image-diff.py
BANK=/home/user/gate-orig-r83/ref
T=/home/user/r108-work/rst; export SOURCE_DATE_EPOCH=1700000000
OUT=/home/user/r108-work/ink-full.txt; : > "$OUT"
score () { python3 "$D" "$1" "$2" --outdir "$T/d" 2>/dev/null \
  | awk -F'\t' 'NF>=6 && $1+0>0 {s+=($4<0?-$4:$4); if($6=="MAJOR")m++} END{printf "%.2f\t%d", s+0, m+0}'; rm -rf "$T/d"; }
while IFS= read -r stem; do
  f=$(grep -F "/$stem.ppt" /home/user/wt-slidesize/dotnet/probes/slides-r107/ppt.list)
  rm -rf "$T"; mkdir -p "$T/base" "$T/head"
  timeout -k 30 600 /home/user/r108-work/bin-base/XRay.Office.Cli render "/home/user/sample-files/$f" --format pdf --outdir "$T/base" >/dev/null 2>&1
  timeout -k 30 600 /home/user/r108-work/bin-head/XRay.Office.Cli render "/home/user/sample-files/$f" --format pdf --outdir "$T/head" >/dev/null 2>&1
  echo -e "$stem\t$(score "$T/base/$stem.pdf" "$BANK/${stem}__ppt.pdf")\t$(score "$T/head/$stem.pdf" "$BANK/${stem}__ppt.pdf")" >> "$OUT"
  rm -rf "$T"
done < /home/user/r108-work/movers.txt
echo FULLDONE >> "$OUT"
