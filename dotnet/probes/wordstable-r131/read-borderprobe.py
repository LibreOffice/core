#!/usr/bin/env python3
"""One rendering of `borderprobe.docx` -> per-arm horizontal lines and row baselines.

    read-borderprobe.py <pdf>

The arm is named by the text in its own cells (`BB00`, `NT10`, ...), so a line is attributed to
the arm whose cells straddle it rather than to a y-range guessed from the layout.
"""
import re
import sys
import pymupdf

doc = pymupdf.open(sys.argv[1])
print('arm\tpage\tkind\ty\tx0\tx1\tthick')
for pno in range(doc.page_count):
    page = doc[pno]
    cells = []
    for w in page.get_text('words'):
        m = re.fullmatch(r'([A-Z]+)(\d+)', w[4])
        if m and len(m.group(2)) >= 2:
            cells.append((m.group(1), int(m.group(2)[:-1]), int(m.group(2)[-1]),
                          w[0], w[1], w[2], w[3]))
    lines = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 'l':
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 0.05 and abs(a.x - b.x) >= 2:
                    lines.append(((a.y + b.y) / 2, min(a.x, b.x), max(a.x, b.x),
                                  d.get('width') or 0.0))
            elif it[0] == 're':
                r = it[1]
                if r.height <= 3 and r.width >= 2:
                    lines.append(((r.y0 + r.y1) / 2, r.x0, r.x1, r.height))
    for arm, row, col, x0, y0, x1, y1 in sorted(cells):
        print(f'{arm}\t{pno+1}\tcell\t{y0:.2f}\t{x0:.2f}\t{x1:.2f}\t{y1-y0:.2f}\tr{row}c{col}')
    for y, x0, x1, t in sorted(lines):
        near = [c for c in cells if abs(c[4] - y) < 40 or abs(c[6] - y) < 40]
        arm = near[0][0] if near else '?'
        print(f'{arm}\t{pno+1}\tline\t{y:.2f}\t{x0:.2f}\t{x1:.2f}\t{t:.3f}')
