#!/usr/bin/env python3
"""Print every horizontal rule 26.2.4.2 drew, out of the PDF's own path operators.

    read-rules.py <pdf> [page]

A rule is a filled `re` no taller than 6 pt and at least 4 pt wide, or a stroked `l` whose two
endpoints share a y.  Nothing is read out of a raster and nothing is inferred from a y-band.
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
                if r.height <= 6.0 and r.width >= 4.0 and k in ('f', 'fs', 's'):
                    out.append((round((r.y0 + r.y1) / 2, 3), round(r.x0, 2), round(r.x1, 2),
                                round(r.height, 3), round(r.y0, 3), round(r.y1, 3), 'fill'))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 0.01 and abs(a.x - b.x) >= 4.0:
                    w = d.get('width') or 0.0
                    out.append((round((a.y + b.y) / 2, 3), round(min(a.x, b.x), 2),
                                round(max(a.x, b.x), 2), round(w, 3),
                                round((a.y + b.y) / 2 - w / 2, 3),
                                round((a.y + b.y) / 2 + w / 2, 3), 'stroke'))
    return sorted(out)


def labels(page):
    return [(round(s['bbox'][1], 2), s['text'].strip())
            for b in page.get_text('dict')['blocks'] for ln in b.get('lines', [])
            for s in ln['spans'] if s['text'].strip()]


doc = pymupdf.open(sys.argv[1])
pages = [int(sys.argv[2]) - 1] if len(sys.argv) > 2 else range(doc.page_count)
for pno in pages:
    page = doc[pno]
    print(f'== page {pno+1} of {doc.page_count}  ({sys.argv[1]})')
    txt = ' '.join(t for _, t in labels(page))
    print(f'   text: {txt[:160]}')
    for y, x0, x1, h, y0, y1, kind in rules(page):
        print(f'   y={y:9.3f}  band {y0:9.3f}..{y1:9.3f}  thick {h:6.3f}  '
              f'x {x0:8.2f}->{x1:8.2f}  len {x1-x0:7.2f}  {kind}')
