#!/usr/bin/env python3
"""How many corpus documents anchor a floating object vertically to the PAGE with an offset?

Counts, per document, `wp:positionV relativeFrom="page"` elements that carry a
`wp:posOffset` (an alignment rather than an offset is unaffected by the base). Walks
word/document.xml plus its headers and footers.
"""
import re, sys, zipfile, pathlib

ROOT = pathlib.Path('/home/user/sample-files')
PV = re.compile(rb'<wp:positionV[^>]*relativeFrom="(\w+)"[^>]*>(.*?)</wp:positionV>', re.S)
docs = 0; tot = 0; rows = []
for p in sorted(ROOT.rglob('*')):
    if not p.is_file() or p.suffix.lower() not in ('.docx', '.docm', '.dotx'):
        continue
    try:
        z = zipfile.ZipFile(p)
    except Exception:
        continue
    n = 0
    for name in z.namelist():
        if not re.match(r'word/(document|header\d*|footer\d*)\.xml$', name):
            continue
        try:
            x = z.read(name)
        except Exception:
            continue
        for m in PV.finditer(x):
            if m.group(1) == b'page' and b'<wp:posOffset>' in m.group(2):
                n += 1
    if n:
        docs += 1; tot += n; rows.append((n, str(p.relative_to(ROOT))))
rows.sort(reverse=True)
print(f'documents {docs}   offsets {tot}')
for n, r in rows[:25]:
    print(f'{n:4d}  {r}')
