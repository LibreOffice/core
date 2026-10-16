#!/bin/bash
# variant.sh <in.fods> <tag>  -- convert a flat ODF back through 26.2.4.2 and print row heights
set -e
IN="$1"; TAG="$2"; SHEET="${3:-6A}"
OUT=/home/user/r98-work/out/$TAG
rm -rf "$OUT"; mkdir -p "$OUT"
/opt/libreoffice26.2/program/soffice --headless \
  -env:UserInstallation=file:///home/user/r98-work/lo-$TAG \
  --convert-to fods --outdir "$OUT" "$IN" >/dev/null 2>&1
F=$(ls "$OUT"/*.fods)
test -s "$F" || { echo "$TAG: EMPTY OUTPUT"; exit 1; }
python3 /home/user/r98-work/bin/fodsrows.py "$F" "$SHEET" | sed -n '1,6p'
python3 /home/user/r98-work/bin/fodsrows.py "$F" "$SHEET" | awk 'NR>1' | \
  python3 -c "
import sys,collections
c=collections.Counter()
for l in sys.stdin:
    p=l.split()
    a,b=p[1].split('-'); h=p[3]
    c[h]+=int(b)-int(a)+1
print('$TAG  height histogram:', dict(c.most_common(6)))
"
