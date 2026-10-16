#!/bin/sh
P=/home/user/libreoffice-core/dotnet/probes/pages-r164
CLI=/home/user/cli-frozen-r164/XRay.Office.Cli
while IFS="$(printf '\t')" read -r k f; do
  mkdir -p "$P/ours/$k" "$P/ours/$k-q"
  $CLI render --outdir "$P/ours/$k" "$f" >> $P/ours/log.txt 2>&1
  XRAY_OFFICE_LIBREOFFICE_QUIRKS=1 $CLI render --outdir "$P/ours/$k-q" "$f" >> $P/ours/log.txt 2>&1
  echo "done $k" >> $P/ours/log.txt
done < $P/docs.tsv
echo ALLDONE >> $P/ours/log.txt
