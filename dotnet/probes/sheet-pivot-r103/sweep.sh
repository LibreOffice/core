#!/bin/bash
# sweep.sh <list> -- render each document at both binaries, %%EOF-check, hash.
set -u
W=/home/user/r103-sheetpivot
printf 'document\tbase\thead\n'
while IFS= read -r doc; do
  [ -z "$doc" ] && continue
  id=$(basename "$doc")
  h=()
  for leg in base head; do
    d=$W/sw/$leg; rm -rf "$d"; mkdir -p "$d"
    SOURCE_DATE_EPOCH=1700000000 dotnet $W/cli-$leg/XRay.Office.Cli.dll render --format pdf --outdir "$d" --quiet "$doc" >/dev/null 2>&1
    f=$(ls "$d"/*.pdf 2>/dev/null | head -1)
    if [ -z "$f" ] || ! tail -c 2048 "$f" | grep -q '%%EOF'; then h+=("FAILED"); else h+=("$(md5sum "$f" | cut -d' ' -f1)"); fi
  done
  printf '%s\t%s\t%s\n' "$id" "${h[0]}" "${h[1]}"
  rm -rf $W/sw
done < "$1"
