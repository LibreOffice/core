#!/usr/bin/env python3
"""Per-page span comparison of two renderings of the same document.

The index pairing in `sensitivity-*.py` counts every span after an inserted line as "moved", which
overstates a rewrap by the length of the document. This reports, per page, whether the span list
differs at all and how many of that page's spans differ once the two pages are paired in order --
so a document whose first page rewraps and whose remaining pages do not reads as 1 page of N.
"""
import sys

import pymupdf


def spans(pdf):
    doc = pymupdf.open(pdf)
    pages = []
    for page in doc:
        out = []
        for b in page.get_text('dict')['blocks']:
            for l in b.get('lines', ()):
                for s in l['spans']:
                    out.append((round(s['bbox'][0], 2), round(s['bbox'][1], 2), s['text']))
        pages.append(out)
    return pages


def main():
    a, b = spans(sys.argv[1]), spans(sys.argv[2])
    print('pages %d vs %d' % (len(a), len(b)))
    differing = 0
    for i, (pa, pb) in enumerate(zip(a, b)):
        if pa == pb:
            continue
        differing += 1
        moved = sum(1 for x, y in zip(pa, pb) if x != y) + abs(len(pa) - len(pb))
        print('page %-3d spans %d vs %d, %d differ' % (i + 1, len(pa), len(pb), moved))
    print('pages that differ at all: %d of %d' % (differing, min(len(a), len(b))))


if __name__ == '__main__':
    main()
