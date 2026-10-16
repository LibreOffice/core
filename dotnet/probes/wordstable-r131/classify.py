#!/usr/bin/env python3
"""Is a bridged gap a TABLE BORDER or a TEXT RULE? Decided on our own layout, not on a raster.

    classify.py <bridges.tsv> <geom-dir>

`gap-census.py` cannot tell the two apart, which is the instrument caveat O82 carries. This can,
because it reads the cell rectangles the layout actually produced (`dump --tsv`) rather than the
page. A bridge is a BORDER only when both hold:

  * the band it sits on is within `TOL` of a cell's top or bottom edge on that page, and
  * BOTH ends of the hole land within `TOL` of a column boundary of the cells on that edge.

The second test is what keeps an underline off the list: a 0.75 pt decoration that happens to run
a quarter of a point under a row's grid line passes the first test and fails the second.
"""
import collections
import pathlib
import sys

TOL = 0.6

rows = [l.split('\t') for l in pathlib.Path(sys.argv[1]).read_text().splitlines()[1:] if l.strip()]
geom = pathlib.Path(sys.argv[2])

cache = {}
def cells(ident):
    if ident not in cache:
        f = geom / f'{ident}.tsv'
        out = collections.defaultdict(list)
        if f.exists():
            for line in f.read_text().splitlines()[1:]:
                p = line.split('\t')
                out[int(p[0])].append(dict(
                    table=int(p[1]), row=int(p[2]), col=int(p[3]),
                    x0=float(p[4]), x1=float(p[5]), y0=float(p[6]), y1=float(p[7]),
                    top=p[8], bottom=p[9]))
        cache[ident] = out
    return cache[ident]

print('ident\tpage\tband_y\tgap_x0\tgap_x1\tgap_pt\tverdict\tdetail')
for r in rows:
    ident, page, y = r[0], int(r[1]), float(r[2])
    gx0, gx1, gpt = float(r[3]), float(r[4]), float(r[5])
    on = cells(ident)[page]
    hits = [c for c in on
            if (abs(c['y0'] - y) <= TOL or abs(c['y1'] - y) <= TOL)
            and c['x0'] < gx1 - 0.5 and c['x1'] > gx0 + 0.5]
    if not hits:
        print(f'{ident}\t{page}\t{y:.2f}\t{gx0:.2f}\t{gx1:.2f}\t{gpt:.2f}\ttext\tno cell edge on this band')
        continue
    xs = sorted({round(v, 3) for c in hits for v in (c['x0'], c['x1'])})
    aligned = (min(abs(v - gx0) for v in xs) <= TOL) and (min(abs(v - gx1) for v in xs) <= TOL)
    side = 'top' if any(abs(c['y0'] - y) <= TOL for c in hits) else 'bottom'
    nones = [c for c in hits if c[side] == 'none']
    cols = ','.join(f"r{c['row']}c{c['col']}" for c in hits)
    if not aligned:
        print(f'{ident}\t{page}\t{y:.2f}\t{gx0:.2f}\t{gx1:.2f}\t{gpt:.2f}\ttext'
              f'\ta cell edge is on this band ({cols}) but the hole does not align to a column boundary')
        continue
    print(f'{ident}\t{page}\t{y:.2f}\t{gx0:.2f}\t{gx1:.2f}\t{gpt:.2f}\tborder'
          f'\t{side} edge of {cols}; {len(nones)} of {len(hits)} state {side}=none')
