#!/bin/bash
out=/home/user/r115-work/class-reach.tsv
: > $out
W=/home/user/r115-work/w2
mkdir -p $W
find /home/user/sample-files/slides -iname '*.ppt' -o -iname '*.pps' -o -iname '*.pot' | sort | while read -r f; do
  rm -f $W/*
  b=$(basename "$f")
  cp "$f" "$W/a.ppt"
  n=$(python3 /home/user/r115-work/declass.py "$f" "$W/b.ppt" 2>/dev/null)
  [ -z "$n" ] && n=ERR
  for v in a b; do
    SOURCE_DATE_EPOCH=1700000000 /opt/libreoffice26.2/program/soffice --headless --norestore \
      -env:UserInstallation=file:///home/user/r115-work/louser --convert-to pdf \
      --outdir $W "$W/$v.ppt" >/dev/null 2>&1
  done
  ra=$(python3 /home/user/r115-work/tj.py "$W/a.pdf" 2>/dev/null || echo -e "0\t0\tERR")
  rb=$(python3 /home/user/r115-work/tj.py "$W/b.pdf" 2>/dev/null || echo -e "0\t0\tERR")
  echo -e "$b\t$n\t$ra\t$rb" >> $out
done
rm -f $W/*
echo DONE >> $out
