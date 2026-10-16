#!/usr/bin/env bash
set -uo pipefail
BASE=/home/user/r110-work/cli-base/XRay.Office.Cli
HEAD=/home/user/wt-slidesize2/dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli
SO=/opt/libreoffice26.2/program/soffice
DIFF=/home/user/wt-slidesize2/.claude/skills/render-comparison/scripts/pdf-image-diff.py
OUT="$2"; T=/home/user/r110-work/inko
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
score () {
  python3 "$DIFF" "$1" "$2" --outdir "$T/d" 2>/dev/null \
    | awk -F'\t' 'NF>=6 && $1+0>0 {s+=($4<0?-$4:$4); if($6=="MAJOR")m++} END{printf "%.2f\t%d", s+0, m+0}'
  rm -rf "$T/d"
}
while IFS= read -r f; do
  [ -n "$f" ] || continue
  b="$(basename "$f")"; stem="${b%.*}"
  rm -rf "$T"; mkdir -p "$T/base" "$T/head" "$T/ref" "$T/u"
  HOME="$T/u" timeout -k 30 900 "$SO" -env:UserInstallation="file://$T/u/x" --headless --convert-to pdf --outdir "$T/ref" "/home/user/corpus-odf/$f" >/dev/null 2>&1
  timeout -k 30 900 "$BASE" render "/home/user/corpus-odf/$f" --format pdf --outdir "$T/base" >/dev/null 2>&1
  timeout -k 30 900 "$HEAD" render "/home/user/corpus-odf/$f" --format pdf --outdir "$T/head" >/dev/null 2>&1
  pb="$T/base/$stem.pdf"; ph="$T/head/$stem.pdf"; ref="$T/ref/$stem.pdf"
  if [ ! -f "$pb" ] || [ ! -f "$ph" ] || [ ! -f "$ref" ]; then echo -e "$stem\tNOLEG" >> "$OUT"; rm -rf "$T"; continue; fi
  echo -e "$stem\tMOVED\t$(score "$pb" "$ref")\t$(score "$ph" "$ref")" >> "$OUT"
  rm -rf "$T"
done < "$1"
echo INKDONE >> "$OUT"
