#!/usr/bin/env bash
# step.sh <pdf> — print the y of three landmark lines on page 3 of a LO render of this doc.
P="$1"
pdftotext -bbox-layout -f 3 -l 3 "$P" - 2>/dev/null | python3 -c "
import sys, re
from xml.etree import ElementTree as ET
NS='{http://www.w3.org/1999/xhtml}'
root=ET.fromstring(sys.stdin.read())
want=['L.L.C. (U912G) for an Order','October 23, 2017','NOTICE IS HEREBY GIVEN','PUBLIC MEETINGS']
for ln in root.iter(NS+'line'):
    t=' '.join(w.text or '' for w in ln.iter(NS+'word')).strip()
    for w in want:
        if t.startswith(w):
            print(f'{float(ln.get(\"yMin\")):9.2f}  {float(ln.get(\"xMin\")):8.2f}  {t[:45]}')
"
