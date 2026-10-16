#!/usr/bin/env bash
# render-ours.sh <outdir>   — render every path in affected.list with the current CLI.
OUT="$1"; mkdir -p "$OUT"
CLI=/home/user/wt-odssheet/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
export SOURCE_DATE_EPOCH=1757289600
cd /home/user/corpus-odf || exit 1
while IFS= read -r rel; do
  base="$(basename "$rel")"; stem="${base%.*}"; ext="${base##*.}"
  d="$OUT/t"; rm -rf "$d"; mkdir -p "$d"
  timeout -k 30 900 "$CLI" render "$rel" --format pdf --outdir "$d" >/dev/null 2>&1
  [ -f "$d/$stem.pdf" ] && mv -f "$d/$stem.pdf" "$OUT/${stem}__${ext,,}.pdf"
done < /home/user/r88-work/affected.list
rm -rf "$OUT/t"
echo RENDER-DONE $(ls "$OUT" | wc -l)
