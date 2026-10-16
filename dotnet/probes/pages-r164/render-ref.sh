#!/bin/sh
P=/home/user/libreoffice-core/dotnet/probes/pages-r164
export PATH=/opt/libreoffice26.2/program:$PATH
soffice --version > $P/ref/VERSION.txt 2>&1
while IFS="$(printf '\t')" read -r k f; do
  mkdir -p "$P/ref/$k" "$P/x/up-$k"
  timeout -k 30 900 soffice --headless -env:UserInstallation=file://$P/x/up-$k \
     --convert-to pdf --outdir "$P/ref/$k" "$f" >> $P/ref/log.txt 2>&1
  echo "done $k" >> $P/ref/log.txt
done < $P/docs.tsv
echo ALLDONE >> $P/ref/log.txt
