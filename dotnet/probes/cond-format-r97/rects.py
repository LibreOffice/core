#!/usr/bin/env python3
"""Every filled rectangle in a PDF, optionally only those of one colour.

The instrument the data-bar geometry was measured with. A bar is one `re f`, so a colour that
nothing else on the page uses names the bars without needing to know where the cells are — which
is why the fixtures state `#2E75B6` rather than a theme slot.

Usage: rects.py <pdf> [#rrggbb]
"""
import sys
import pymupdf

doc = pymupdf.open(sys.argv[1])
want = sys.argv[2] if len(sys.argv) > 2 else None
for pno, page in enumerate(doc, 1):
    for d in page.get_drawings():
        if d['type'] not in ('f', 'fs') or d.get('fill') is None:
            continue
        hexv = '#%02x%02x%02x' % tuple(int(round(c * 255)) for c in d['fill'])
        if want and hexv != want:
            continue
        r = d['rect']
        print(f"p{pno} {hexv} x {r.x0:8.2f}-{r.x1:8.2f} y {r.y0:8.2f}-{r.y1:8.2f} "
              f"w {r.width:7.2f} h {r.height:6.2f}")
