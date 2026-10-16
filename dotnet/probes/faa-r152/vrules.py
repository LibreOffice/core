#!/usr/bin/env python3
"""Vertical rules on one page of a PDF: x, y0, y1, thickness."""
import sys, pymupdf
doc = pymupdf.open(sys.argv[1]); page = doc[int(sys.argv[2]) - 1]
ylo = float(sys.argv[3]) if len(sys.argv) > 3 else -1e9
yhi = float(sys.argv[4]) if len(sys.argv) > 4 else 1e9
out = []
for d in page.get_drawings():
    k = d.get('type', '')
    for it in d['items']:
        if it[0] == 're':
            r = it[1]
            if r.width <= 3.0 and r.height >= 2.0 and k in ('f', 'fs', 's'):
                out.append(((r.x0 + r.x1) / 2, r.y0, r.y1, r.width, 'fill'))
        elif it[0] == 'l' and k in ('s', 'fs'):
            a, b = it[1], it[2]
            if abs(a.x - b.x) <= 3.0 and abs(a.y - b.y) >= 2.0:
                out.append(((a.x + b.x) / 2, min(a.y, b.y), max(a.y, b.y), d.get('width') or 0.0, 'stroke'))
for x, y0, y1, w, k in sorted(out):
    if y0 <= yhi and y1 >= ylo:
        print(f'x={x:8.2f}  y {y0:8.2f} -> {y1:8.2f}  thick {w:.4f}  {k}')
