#!/usr/bin/env python3
"""Print every drawn rule on one page of a PDF, both shapes (C16), sorted by y then x.

    show-rules.py <pdf> <1-based page> [ymin ymax]
"""
import sys
import pymupdf

def rules(page):
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 3.0 and r.width >= 2.0 and k in ('f', 'fs', 's'):
                    out.append((round((r.y0 + r.y1) / 2, 2), round(r.x0, 2), round(r.x1, 2),
                                round(r.height, 4), 'fill'))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 3.0 and abs(a.x - b.x) >= 2.0:
                    w = d.get('width') or 0.0
                    out.append((round((a.y + b.y) / 2, 2), round(min(a.x, b.x), 2),
                                round(max(a.x, b.x), 2), round(w, 4), 'stroke'))
    return sorted(out)

path, pno = sys.argv[1], int(sys.argv[2])
ymin = float(sys.argv[3]) if len(sys.argv) > 3 else -1e9
ymax = float(sys.argv[4]) if len(sys.argv) > 4 else 1e9
doc = pymupdf.open(path)
page = doc[pno - 1]
total = 0.0
for y, x0, x1, h, kind in rules(page):
    if ymin <= y <= ymax:
        print(f'y={y:8.2f}  x {x0:8.2f} -> {x1:8.2f}  len {x1-x0:8.2f}  thick {h:.4f}  {kind}')
        total += x1 - x0
print(f'-- {path} page {pno}: total cover {total:.2f} pt')
