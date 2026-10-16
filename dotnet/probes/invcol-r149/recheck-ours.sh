#!/usr/bin/env bash
# The parent rebuilt XRay.Office.Cli at 21:58:14 while this round's sweep was running
# (sweep-all/cli-mtime.txt records both stamps). A comparison that spans a rebuild is
# void unless the rebuild is shown not to have changed the renderings, so re-render
# OUR half with the binary as it now stands and compare byte for byte.
set -u
OUT="$(cd "$1" && pwd)"; CLI=/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
mkdir -p "$OUT/ours-recheck"
stat -c "CLI %y %s" "$CLI" > "$OUT/recheck-mtime.txt"
i=0
while IFS= read -r f; do
  i=$((i+1)); [ $((i % 2)) -eq 0 ] || continue
  stem="$(basename "${f%.*}")"
  [ -f "$OUT/ours-recheck/$stem.pdf" ] && continue
  rm -rf "$OUT/rc0"; mkdir -p "$OUT/rc0"
  SOURCE_DATE_EPOCH=0 timeout -k 30 600 "$CLI" render "$f" --format pdf --outdir "$OUT/rc0" >/dev/null 2>&1
  [ -f "$OUT/rc0/$stem.pdf" ] && mv -f "$OUT/rc0/$stem.pdf" "$OUT/ours-recheck/$stem.pdf"
done < "$2" &
j=0
while IFS= read -r f; do
  j=$((j+1)); [ $((j % 2)) -eq 1 ] || continue
  stem="$(basename "${f%.*}")"
  [ -f "$OUT/ours-recheck/$stem.pdf" ] && continue
  rm -rf "$OUT/rc1"; mkdir -p "$OUT/rc1"
  SOURCE_DATE_EPOCH=0 timeout -k 30 600 "$CLI" render "$f" --format pdf --outdir "$OUT/rc1" >/dev/null 2>&1
  [ -f "$OUT/rc1/$stem.pdf" ] && mv -f "$OUT/rc1/$stem.pdf" "$OUT/ours-recheck/$stem.pdf"
done < "$2" &
wait
stat -c "CLI %y %s" "$CLI" >> "$OUT/recheck-mtime.txt"
echo done
