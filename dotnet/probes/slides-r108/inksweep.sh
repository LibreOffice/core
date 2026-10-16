#!/usr/bin/env bash
# Both legs of the 51-document .ppt column against the banked 26.2.4.2 renderings.
set -uo pipefail
BASE=/home/user/r108-work/bin-base/XRay.Office.Cli
HEAD=/home/user/r108-work/bin-head/XRay.Office.Cli
BANK=/home/user/gate-orig-r83/ref
DIFF=/home/user/wt-slidesize/.claude/skills/render-comparison/scripts/pdf-image-diff.py
OUT=/home/user/r108-work/ink.txt
T=/home/user/r108-work/inkt
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
score () {  # <pdf> <ref>  -> "sum|ink|%  MAJOR"
  python3 "$DIFF" "$1" "$2" --outdir "$T/d" --quiet 2>/dev/null \
    | awk -F'\t' 'NF>=6 && $1+0>0 {s+=($4<0?-$4:$4); if($6=="MAJOR")m++} END{printf "%.2f\t%d", s+0, m+0}'
  rm -rf "$T/d"
}
while IFS= read -r f; do
  [ -n "$f" ] || continue
  b="$(basename "$f")"; stem="${b%.*}"
  rm -rf "$T"; mkdir -p "$T/base" "$T/head"
  timeout -k 30 600 "$BASE" render "/home/user/sample-files/$f" --format pdf --outdir "$T/base" >/dev/null 2>&1
  timeout -k 30 600 "$HEAD" render "/home/user/sample-files/$f" --format pdf --outdir "$T/head" >/dev/null 2>&1
  pb="$T/base/$stem.pdf"; ph="$T/head/$stem.pdf"; ref="$BANK/${stem}__ppt.pdf"
  if [ ! -f "$pb" ] || [ ! -f "$ph" ]; then echo -e "$stem\tFAILED" >> "$OUT"; rm -rf "$T"; continue; fi
  mb=$(md5sum "$pb" | cut -c1-32); mh=$(md5sum "$ph" | cut -c1-32)
  gb=$(pdftotext "$pb" - 2>/dev/null | tr -cd '[:alnum:]' | wc -c)
  gh=$(pdftotext "$ph" - 2>/dev/null | tr -cd '[:alnum:]' | wc -c)
  nb=$(pdfinfo "$pb" 2>/dev/null | awk '/^Pages/{print $2}')
  nh=$(pdfinfo "$ph" 2>/dev/null | awk '/^Pages/{print $2}')
  sb=$(score "$pb" "$ref")
  if [ "$mb" = "$mh" ]; then
    echo -e "$stem\tSAME\t$nb\t$gb\t$sb\t$sb" >> "$OUT"
  else
    sh=$(score "$ph" "$ref")
    echo -e "$stem\tMOVED\t$nb/$nh\t$gb/$gh\t$sb\t$sh" >> "$OUT"
  fi
  rm -rf "$T"
done < /home/user/wt-slidesize/dotnet/probes/slides-r107/ppt.list
echo INKDONE >> "$OUT"
