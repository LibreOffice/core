#!/usr/bin/env python3
"""How many of a leg's stroke widths find a reference stroke of the same width.

Restricted to the pages where the two legs differ at all, so the score is about
what changed and not about what a document already drew.  A width matches when it
is within 0.02 pt of an unclaimed reference stroke's; the pool is consumed, so two
of ours cannot both match one of theirs.

Usage: widthmatch.py <base-dir> <head-dir> <reference-dir>
"""
import collections
import os
import sys
import pymupdf


def widths(page):
    return collections.Counter(
        round(p.get('width') or 0, 3)
        for p in page.get_drawings() if p['type'] in ('s', 'fs'))


def matched(ours, reference, tolerance=0.02):
    pool = [w for w, n in reference.items() for _ in range(n)]
    hits = 0
    for width, n in ours.items():
        for _ in range(n):
            near = [w for w in pool if abs(w - width) <= tolerance]
            if near:
                pool.remove(near[0])
                hits += 1
    return hits


def main():
    base, head, ref = sys.argv[1], sys.argv[2], sys.argv[3]
    pages = ours = theirs = 0
    hit_base = hit_head = 0
    print('doc\tpage\tbase_matched\thead_matched\tours\tref')
    for name in sorted(n for n in os.listdir(base) if n.endswith('.pdf')):
        b, h, r = (pymupdf.open(os.path.join(d, name)) for d in (base, head, ref))
        for i in range(b.page_count):
            wb, wh = widths(b[i]), widths(h[i])
            if wb == wh:
                continue
            wr = widths(r[i])
            pages += 1
            mb, mh = matched(wb, wr), matched(wh, wr)
            hit_base += mb
            hit_head += mh
            ours += sum(wh.values())
            theirs += sum(wr.values())
            print(f'{name[:40]}\t{i + 1}\t{mb}\t{mh}\t{sum(wh.values())}\t{sum(wr.values())}')
    print(f'TOTAL pages {pages}\tbase matched {hit_base}\thead matched {hit_head}'
          f'\tours {ours}\treference {theirs}')


if __name__ == '__main__':
    main()
