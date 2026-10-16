"""How well a rendering's stroked polylines agree with the reference's, connector by connector.

A .ppt connector is a stroked open path.  Ours and the reference's are paired on their END
POINTS -- which are the shape's own bounding-rectangle corners and do not depend on the
adjustment -- and scored on the INTERIOR vertices, which are exactly what the adjustment moves.
"""
import re, sys
import numpy as np
from refpage import page_stream

def polylines(pdf, npages):
    out = {}
    for i in range(npages):
        s, _ = page_stream(pdf, i)
        rows = []
        for m in re.finditer(r'((?:-?[\d.]+\s+-?[\d.]+\s+m)(?:\s+-?[\d.]+\s+-?[\d.]+\s+[lc])+)\s*S', s):
            v = [float(x) for x in re.findall(r'-?[\d.]+', m.group(1))]
            pts = list(zip(v[0::2], v[1::2]))
            if 3 <= len(pts) <= 12: rows.append(pts)
        out[i] = rows
    return out

def npages(pdf):
    _, n = page_stream(pdf, 0)
    return n

def score(mine, ref):
    paired = agree = 0
    resid = []
    for i, rows in mine.items():
        for pts in rows:
            best = None
            for other in ref.get(i, []):
                d = max(abs(pts[0][0]-other[0][0]), abs(pts[0][1]-other[0][1]),
                        abs(pts[-1][0]-other[-1][0]), abs(pts[-1][1]-other[-1][1]))
                if d <= 6.0 and len(other) == len(pts) and (best is None or d < best[0]):
                    best = (d, other)
            if best is None: continue
            paired += 1
            inner = max(max(abs(a[0]-b[0]), abs(a[1]-b[1]))
                        for a, b in zip(pts[1:-1], best[1][1:-1])) if len(pts) > 2 else 0.0
            resid.append(inner)
            if inner <= 1.0: agree += 1
    return paired, agree, resid

for n in sys.argv[1:]:
    ref = polylines(f'conn-ref/{n}/{n}.pdf', npages(f'conn-ref/{n}/{n}.pdf'))
    for tag in ('before', 'after'):
        mine = polylines(f'conn-{tag}/{n}/{n}.pdf', npages(f'conn-{tag}/{n}/{n}.pdf'))
        p, a, r = score(mine, ref)
        print(f'{n[:40]:<42}{tag:<8}paired {p:>4}  within 1pt {a:>4}  '
              f'median {np.median(r) if r else 0:.3f}  mean {np.mean(r) if r else 0:.3f}')
