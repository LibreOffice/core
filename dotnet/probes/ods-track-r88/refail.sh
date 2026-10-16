#!/usr/bin/env bash
# Render each of odf-gate-r80's 11 .ods ref-failed rows through 26.2.4.2 ALONE,
# with a 900 s bound, and record wall time and page count.
REF=/opt/libreoffice26.2/program/soffice
OUT=/home/user/r88-work/refail
mkdir -p "$OUT"
: > "$OUT/times.tsv"
while IFS= read -r rel; do
  f="/home/user/corpus-odf/$rel"
  [ -f "$f" ] || { echo -e "$rel\tMISSING\t-\t-" >> "$OUT/times.tsv"; continue; }
  rm -rf "$OUT/t" "$OUT/prof"; mkdir -p "$OUT/t"
  s=$(date +%s)
  timeout -k 30 900 "$REF" -env:UserInstallation="file://$OUT/prof" \
      --headless --norestore --convert-to pdf --outdir "$OUT/t" "$f" >/dev/null 2>&1
  rc=$?
  e=$(( $(date +%s) - s ))
  pdf=$(ls "$OUT/t"/*.pdf 2>/dev/null | head -1)
  if [ -n "$pdf" ]; then
    pg=$(pdfinfo "$pdf" 2>/dev/null | awk '/^Pages:/{print $2}')
    ch=$(pdftotext "$pdf" - 2>/dev/null | tr -cd '[:alnum:]' | wc -c)
  else pg=-; ch=-; fi
  echo -e "$rel\trc=$rc\t${e}s\tpages=$pg\tglyphs=$ch" >> "$OUT/times.tsv"
  echo "done $rel rc=$rc ${e}s pages=$pg"
done < /home/user/r88-work/refail.list
echo REFAIL-DONE
