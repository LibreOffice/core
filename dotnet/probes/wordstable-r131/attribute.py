#!/usr/bin/env python3
"""Attribute every horizontal rule of a borderprobe rendering to the arm it belongs to.

    attribute.py <pdf>

An arm is named by the text in its own cells, so a line goes to the arm of the nearest cell
rather than to a y-range guessed from the layout -- which matters because two arms can be
twelve points apart.
"""
import re
import sys
import pymupdf

doc = pymupdf.open(sys.argv[1])
print('arm\tpage\ty\tx0\tx1\tthick')
for pno in range(doc.page_count):
    pg = doc[pno]
    cells = []
    for w in pg.get_text('words'):
        m = re.fullmatch(r'([A-Z]+)(\d+)', w[4])
        if m and len(m.group(2)) >= 2:
            cells.append((w[1], w[3], m.group(1)))
    lines = []
    for d in pg.get_drawings():
        for it in d['items']:
            if it[0] == 'l':
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 0.05 and abs(a.x - b.x) >= 2:
                    lines.append(((a.y + b.y) / 2, min(a.x, b.x), max(a.x, b.x), d.get('width') or 0))
            elif it[0] == 're':
                r = it[1]
                if r.height <= 3 and r.width >= 2:
                    lines.append(((r.y0 + r.y1) / 2, r.x0, r.x1, r.height))
    for y, x0, x1, t in sorted(lines):
        arm = min(cells, key=lambda c: min(abs(c[0] - y), abs(c[1] - y)))[2] if cells else '?'
        print(f'{arm}\t{pno+1}\t{y:.2f}\t{x0:.2f}\t{x1:.2f}\t{t:.3f}')
