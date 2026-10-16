#!/usr/bin/env python3
"""Where 26.2.4.2 puts the invoice under each value of style:table-centering.

Reports, per printed page, the minimum span x0 and minimum span y0 -- the top-left
corner of the drawn block -- for each variant, against the as-found render.
"""
import glob, pymupdf, collections, sys

def corners(path):
    out = {}
    for pn, pg in enumerate(pymupdf.open(path)):
        xs, ys = [], []
        for b in pg.get_text('dict')['blocks']:
            for l in b.get('lines', []):
                for s in l['spans']:
                    if s['text'].strip():
                        xs.append(s['bbox'][0]); ys.append(s['bbox'][1])
        if xs:
            out[pn] = (min(xs), min(ys))
    return out

base = corners(glob.glob('render/ref/*.pdf')[0])
ours = corners(glob.glob('render/ours/*.pdf')[0])
print(f'{"variant":<12} {"page":>4} {"min x0":>9} {"min y0":>9}   {"dx vs as-found":>15} {"dy":>7}')
for name in ('asfound', 'none', 'vertical', 'both'):
    c = corners(f'variants/ref/invoice-{name}.pdf')
    for pn in sorted(c):
        bx, by = base.get(pn, (float('nan'),) * 2)
        print(f'{name:<12} {pn:>4} {c[pn][0]:9.2f} {c[pn][1]:9.2f}   '
              f'{c[pn][0]-bx:15.2f} {c[pn][1]-by:7.2f}')
print()
for pn in sorted(ours):
    print(f'{"OURS":<12} {pn:>4} {ours[pn][0]:9.2f} {ours[pn][1]:9.2f}   '
          f'{ours[pn][0]-base[pn][0]:15.2f} {ours[pn][1]-base[pn][1]:7.2f}')
