#!/usr/bin/env python3
"""Census of horizontal boundaries whose columns carry BAND STACKS OF DIFFERENT EXTENT.

    extent-census.py <pairs.tsv> <out-prefix>

This supersedes both `tablerow-r146/census-pdf.py` and `tbalign-r147/mixed-lines.py`.

Why extent and not thickness.  A `w:val="double"` border of width w is emitted as THREE
rectangles of w/3 (`BorderRules.cs`:203 maps "double" => 3, and 26.2.4.2 draws the same three -- r146 §2), so a boundary
where some columns are double and others single carries one thickness everywhere and is still a
boundary this rule cannot draw with one centred band.  `150_5335_5a.doc` p28 is exactly that:
four 0.5 pt runs at top 292.300 of which two continue to 293.300 and two do not.  A thickness
census scores it 0; it moved in round 147's sweep, correctly.

So: a boundary is a maximal x-abutting chain of segments sharing one top edge; each member's
EXTENT is how far contiguous ink continues downwards from that top over the same x; the
boundary is MIXED when two members have different extents.  For a single border the extent is
the thickness, so this contains the thickness rule.
"""
import collections
import sys

import pymupdf

MAXTH = 6.0
GAP = 1.0
EPS = 0.02
MINLEN = 12.0   # pt; a segment shorter than this is line-art, not a column band


def segs(page):
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're' and k in ('f', 'fs', 's'):
                r = it[1]
                if r.height <= MAXTH and r.width >= 4.0 and r.width > r.height:
                    out.append((r.y0, r.x0, r.x1, r.height))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                w = d.get('width') or 0.0
                if abs(a.y - b.y) <= 0.01 and abs(a.x - b.x) >= 4.0 and w <= MAXTH:
                    out.append(((a.y + b.y) / 2 - w / 2, min(a.x, b.x), max(a.x, b.x), w))
    return out


def score(H):
    bytop = collections.defaultdict(set)
    for top, x0, x1, th in H:
        if x1 - x0 < MINLEN:
            continue
        bytop[round(top, 2)].add((round(x0, 2), round(x1, 2), round(th, 3)))

    tops = sorted(bytop)

    def extent(top, x0, x1, own):
        """How far a band STACK continues down from `top` over x0..x1.

        A `w:val="double"` border is line / gap / line, each of `w:sz/8` (r146 measured
        26.2.4.2 drawing three equal bands at 7 of 7 sizes), so the stack must be allowed to
        jump a gap of at most one band thickness -- and no more, or two adjacent boundaries a
        short row apart merge.  The continuation must also cover at least half the narrower of
        the two x runs: two bands of different width overlap by up to 1.5 pt at the mitre
        (measured on `B11. TE.CAO.00129`) and a bare overlap test lets a neighbour's thickness
        leak in and hides the very boundary being looked for.
        """
        y, guard = top + own, 0
        while guard < 12:
            guard += 1
            nxt = None
            for t in tops:
                if t < y - 0.02:
                    continue
                if t > y + own + 0.05:
                    break
                for (a0, a1, th) in bytop[t]:
                    ov = min(a1, x1) - max(a0, x0)
                    if ov > 0.5 * min(x1 - x0, a1 - a0) and (nxt is None or t + th > nxt):
                        nxt = t + th
            if nxt is None:
                break
            y = nxt
        return round(y - top, 3)

    mix = same = 0
    for top, v in bytop.items():
        v = sorted(v)
        cur = [v[0]]
        chunks = []
        for s in v[1:]:
            if s[0] - max(t[1] for t in cur) <= GAP:
                cur.append(s)
            else:
                chunks.append(cur); cur = [s]
        chunks.append(cur)
        for c in chunks:
            if len(c) < 2:
                continue
            ex = {extent(top, s[0], s[1], s[2]) for s in c}
            if max(ex) - min(ex) > EPS:
                mix += 1
            else:
                same += 1
    return mix, same


def main():
    rows = []
    for line in open(sys.argv[1]):
        rel, pdf = line.rstrip('\n').split('\t')
        doc = pymupdf.open(pdf)
        m = s = 0
        for p in range(doc.page_count):
            a, b = score(segs(doc[p]))
            m += a; s += b
        doc.close()
        rows.append((rel, m, s))
        print('.', end='', flush=True)
    print()
    with open(sys.argv[2] + '.tsv', 'w') as f:
        f.write('path\tmixed_extent\tsame_extent_base\n')
        for r in rows:
            f.write('%s\t%d\t%d\n' % r)
    print('documents read                       %4d' % len(rows))
    print('  >=1 mixed-extent boundary          %4d docs  %6d boundaries'
          % (sum(1 for r in rows if r[1] > 0), sum(r[1] for r in rows)))
    print('  >=1 one-extent column boundary     %4d docs  %6d boundaries'
          % (sum(1 for r in rows if r[2] > 0), sum(r[2] for r in rows)))


if __name__ == '__main__':
    main()
