#!/usr/bin/env python3
"""Per page, how far each leg's drawn row edges are from the reference's.

A `.ppt` table's rules are the only horizontal hairlines on these pages, so the
grid can be read straight out of a PDF: every stroked or filled path less than
0.6 pt tall, at the y of its own centre.  Pages where a leg draws a different
number of them are skipped rather than paired, because the pairing would be
meaningless -- and a page where every leg already agrees with the reference to
within 0.05 pt is not printed.

Guard worth keeping: a renderer may draw a rule as a thin FILL rather than a
stroke.  Both of these do stroke them, which is why the item counts are printed
by `tablecompare.py`; the filter here takes either kind for the same reason.

Usage: rowedges.py <leg-dir> [<leg-dir> ...] <reference-dir>
"""
import os
import sys
import pymupdf


def horizontals(page):
    out = []
    for p in page.get_drawings():
        if p['type'] in ('s', 'fs'):
            r = p['rect']
            if r.height < 0.6:
                out.append(round(r.y0 + r.height / 2, 2))
    return sorted(out)


def main():
    legs = sys.argv[1:]
    ref = legs[-1]
    score = {leg: 0.0 for leg in legs[:-1]}
    pages = 0

    for name in sorted(n for n in os.listdir(legs[0]) if n.endswith('.pdf')):
        opened = {leg: pymupdf.open(os.path.join(leg, name)) for leg in legs}
        for i in range(opened[legs[0]].page_count):
            reference = horizontals(opened[ref][i])
            if not reference:
                continue
            drawn = {leg: horizontals(opened[leg][i]) for leg in legs[:-1]}
            if any(len(v) != len(reference) for v in drawn.values()):
                continue
            if all(max(abs(a - b) for a, b in zip(v, reference)) < 0.05
                   for v in drawn.values()):
                continue

            pages += 1
            row = [name[:34], str(i + 1)]
            for leg in legs[:-1]:
                worst = max(abs(a - b) for a, b in zip(drawn[leg], reference))
                score[leg] += worst
                row.append(f'{leg}:{worst:.2f}')
            print('\t'.join(row))

    print('pages', pages, {leg: round(v, 2) for leg, v in score.items()})


if __name__ == '__main__':
    main()
