#!/usr/bin/env python3
"""Every stroked straight segment a PDF draws, read from the page's path operators.

PyMuPDF's get_drawings() replays the content stream's path operators, so this is the
drawing the file states and not a raster of it. Only stroked items are reported; a filled
rectangle used as a rule would show as 'f' and is reported separately.
"""
import sys, pymupdf, collections

def segs(path):
    doc = pymupdf.open(path)
    out = []
    for pno, page in enumerate(doc, 1):
        for d in page.get_drawings():
            if d['type'] not in ('s', 'fs'):
                continue
            w = d.get('width') or 0
            col = d.get('color')
            for item in d['items']:
                if item[0] == 'l':
                    p, q = item[1], item[2]
                    out.append((pno, p.x, p.y, q.x, q.y, w, col))
                elif item[0] == 're':
                    r = item[1]
                    out.append((pno, r.x0, r.y0, r.x1, r.y1, w, col, 're'))
    return out, doc.page_count

if __name__ == '__main__':
    for f in sys.argv[1:]:
        out, np = segs(f)
        print(f'== {f}: {np} pages, {len(out)} stroked items')
        c = collections.Counter()
        for s in out:
            kind = 'rect' if len(s) > 7 else ('horiz' if abs(s[2]-s[4]) < 0.01 else
                    ('vert' if abs(s[1]-s[3]) < 0.01 else 'diag'))
            c[kind] += 1
        print('   ', dict(c))
