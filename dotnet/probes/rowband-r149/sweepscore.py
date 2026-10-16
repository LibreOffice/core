#!/usr/bin/env python3
"""Score a sweep leg's horizontal table bands against the banked 26.2.4.2 renderings.

One reader for both halves (see `score.py`'s remark). Two figures per document, because a
whole-page displacement and a wrong band SHAPE are different defects and only the second is what
this round is about:

  absolute -- how many of the reference's bands we put within `tol` of where it put them;
  shape    -- the same after removing each page's median displacement.

A page whose band counts differ is not scored at all: pairing across a count mismatch invents its
own answer. Those pages are counted and reported, because a change that fixes the shape by
dropping a band would otherwise look like an improvement.
"""
import collections
import hashlib
import pathlib
import statistics
import sys

import pymupdf

TOL = 0.1


def bands(path):
    out = []
    for number, page in enumerate(pymupdf.open(path)):
        for drawing in page.get_drawings():
            for item in drawing['items']:
                if item[0] == 're':
                    r = item[1]
                    if r.width >= 4 and r.height <= 6:
                        out.append((number, round(r.y0, 3), round(r.y1, 3)))
                elif item[0] == 'l':
                    a, b = item[1], item[2]
                    if abs(a.y - b.y) <= 0.01 and abs(a.x - b.x) >= 4:
                        w = drawing.get('width') or 0.0
                        out.append((number, round(a.y - w / 2, 3), round(a.y + w / 2, 3)))
    pages = collections.defaultdict(list)
    for b in out:
        pages[b[0]].append(b)
    for page in pages.values():
        page.sort()
    return pages


def compare(ref, ours):
    absolute = shape = total = mismatch = 0
    for page in ref:
        a, b = ref[page], ours.get(page, [])
        if len(a) != len(b):
            mismatch += len(a)
            continue
        if not a:
            continue
        total += len(a)
        d = [y[1] - x[1] for x, y in zip(a, b)]
        absolute += sum(1 for v in d if abs(v) <= TOL)
        shift = statistics.median(d)
        shape += sum(1 for v in d if abs(v - shift) <= TOL)
    return absolute, shape, total, mismatch


def main():
    leg = pathlib.Path(sys.argv[1])
    refs = {p.stem: p for p in pathlib.Path(sys.argv[2]).rglob('*.pdf')}
    docs = [line.split('\t') for line in
            pathlib.Path(sys.argv[3]).read_text().splitlines() if line]

    tot = [0, 0, 0, 0]
    scored = 0
    for family, ext, path in docs:
        stem = pathlib.PurePath(path).stem
        if stem not in refs:
            continue
        key = hashlib.md5(('/home/user/sample-files/' + path).encode()).hexdigest()[:12]
        mine = list((leg / key).glob('*.pdf'))
        if not mine:
            continue
        a, s, t, m = compare(bands(refs[stem]), bands(mine[0]))
        scored += 1
        for i, v in enumerate((a, s, t, m)):
            tot[i] += v

    print('%-6s documents scored %3d   bands %5d (+%d on count-mismatched pages)'
          % (leg.name, scored, tot[2], tot[3]))
    print('%-6s   absolute within %.2f pt: %5d / %-5d  %5.1f %%'
          % ('', TOL, tot[0], tot[2], 100.0 * tot[0] / max(1, tot[2])))
    print('%-6s   shape    within %.2f pt: %5d / %-5d  %5.1f %%'
          % ('', TOL, tot[1], tot[2], 100.0 * tot[1] / max(1, tot[2])))


if __name__ == '__main__':
    main()
