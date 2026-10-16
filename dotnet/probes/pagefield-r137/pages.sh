#!/bin/sh
# usage: pages.sh file.pdf [regex]
f="$1"; re="${2:-[Pp]age}"
n=$(pdfinfo "$f" 2>/dev/null | awk '/^Pages:/{print $2}')
echo "== $f ($n pages)"
i=1
while [ "$i" -le "$n" ]; do
  t=$(pdftotext -f "$i" -l "$i" -layout "$f" - 2>/dev/null | grep -E "$re" | tr '\n' '|' )
  echo "  p$i: $t"
  i=$((i+1))
done
