#!/usr/bin/env python3
"""Count w:pict / v:imagedata occurrences in every DOCX of a corpus track."""
import sys, zipfile, re, os

root = sys.argv[1]
hits = []
for dirpath, _, names in os.walk(root):
    for n in sorted(names):
        if not n.lower().endswith(('.docx', '.docm', '.dotx')):
            continue
        p = os.path.join(dirpath, n)
        try:
            z = zipfile.ZipFile(p)
        except Exception:
            continue
        total_pict = 0
        total_img = 0
        for item in z.namelist():
            if not item.endswith('.xml') or not item.startswith('word/'):
                continue
            try:
                s = z.read(item).decode('utf8', 'replace')
            except Exception:
                continue
            total_pict += s.count('<w:pict')
            total_img += len(re.findall(r'<v:imagedata', s))
        if total_pict:
            hits.append((os.path.relpath(p, root), total_pict, total_img))

for h in sorted(hits, key=lambda x: -x[2]):
    print(f'{h[1]:5d} pict {h[2]:5d} imagedata  {h[0]}')
print(f'--- {len(hits)} documents with w:pict, '
      f'{sum(1 for h in hits if h[2])} with v:imagedata')
