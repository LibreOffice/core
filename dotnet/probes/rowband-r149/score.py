#!/usr/bin/env python3
"""Score this tree's horizontal table bands against 26.2.4.2's, fixture by fixture.

Both halves are read by ONE reader, because round 148 spent a whole agent reconciling two
instruments that had each been written for one side.

A whole-table displacement and a wrong band SHAPE are different defects and the second is what
O83/O84/O85 are about, so the two are reported separately: the bands of a page are paired in
sorted order, the page's median (ours - reference) top offset is taken as the displacement, and
the RESIDUAL after removing it is the shape error. A page whose band counts differ is reported
as such and not scored -- a pairing across a count mismatch invents its own answer.
"""
import argparse
import collections
import pathlib
import statistics

import pymupdf


def bands(path):
    """Every horizontal band, as (page, top, bottom, x0, x1) rounded to 1/1000."""
    out = []
    for number, page in enumerate(pymupdf.open(path)):
        for drawing in page.get_drawings():
            for item in drawing['items']:
                if item[0] == 're':
                    r = item[1]
                    if r.width >= 4 and r.height <= 6:
                        out.append((number, round(r.y0, 3), round(r.y1, 3),
                                    round(r.x0, 3), round(r.x1, 3)))
                elif item[0] == 'l':
                    a, b = item[1], item[2]
                    if abs(a.y - b.y) <= 0.01 and abs(a.x - b.x) >= 4:
                        w = drawing.get('width') or 0.0
                        out.append((number, round(a.y - w / 2, 3), round(a.y + w / 2, 3),
                                    round(min(a.x, b.x), 3), round(max(a.x, b.x), 3)))
    return out


def by_page(rows):
    pages = collections.defaultdict(list)
    for b in rows:
        pages[b[0]].append(b)
    for page in pages.values():
        page.sort(key=lambda b: (b[1], b[3]))
    return pages


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ref')
    ap.add_argument('ours')
    ap.add_argument('--tol', type=float, default=0.05)
    args = ap.parse_args()

    for path in sorted(pathlib.Path(args.ref).glob('*.pdf')):
        theirs = pathlib.Path(args.ours) / path.name
        if not theirs.exists():
            continue
        r, o = by_page(bands(path)), by_page(bands(theirs))

        mismatched = []
        offsets, residuals, thick = [], [], []
        for page in sorted(set(r) | set(o)):
            a, b = r.get(page, []), o.get(page, [])
            if len(a) != len(b):
                mismatched.append('p%d %d/%d' % (page, len(a), len(b)))
                continue
            if not a:
                continue
            d = [y[1] - x[1] for x, y in zip(a, b)]
            shift = statistics.median(d)
            offsets.append(shift)
            residuals += [abs(v - shift) for v in d]
            thick += [abs((y[2] - y[1]) - (x[2] - x[1])) for x, y in zip(a, b)]

        if not residuals:
            print('%-20s NOT SCOREABLE  %s' % (path.stem[:20], ' '.join(mismatched)))
            continue
        good = sum(1 for v in residuals if v <= args.tol)
        print('%-20s shift %7.3f..%-7.3f   shape %3d/%-3d within %.2f (worst %6.3f)   '
              'thickness worst %6.3f%s'
              % (path.stem[:20], min(offsets), max(offsets), good, len(residuals),
                 args.tol, max(residuals), max(thick),
                 ('   MISMATCHED ' + ' '.join(mismatched)) if mismatched else ''))


if __name__ == '__main__':
    main()
