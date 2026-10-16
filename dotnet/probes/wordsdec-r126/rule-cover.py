#!/usr/bin/env python3
"""Total merged rule cover per document, ours against the reference.

    rule-cover.py <ours-dir> <ref-dir> <out.tsv>

Whole documents rather than selected pages, so a fix that closes a gap in one place and invents
ink in another cannot hide inside a page filter. Merged per y-band and counted on both shapes
(C16), and the reference's own duplicate strokes -- it draws some rules twice -- are collapsed
by the same merge, which is why this is not a stroke count.
"""
import pathlib
import sys

import pymupdf

TOL_Y = 1.2


def cover(page):
    bands = {}
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 3.0 and r.width >= 0.5 and k in ('f', 'fs', 's'):
                    bands.setdefault(round((r.y0 + r.y1) / 2 / TOL_Y), []).append((r.x0, r.x1))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 3.0 and abs(a.x - b.x) >= 0.5:
                    bands.setdefault(round((a.y + b.y) / 2 / TOL_Y), []).append(
                        (min(a.x, b.x), max(a.x, b.x)))
    total = 0.0
    for spans in bands.values():
        spans.sort()
        cur = None
        for x0, x1 in spans:
            if cur and x0 <= cur[1] + 0.5:
                cur[1] = max(cur[1], x1)
            else:
                if cur:
                    total += cur[1] - cur[0]
                cur = [x0, x1]
        if cur:
            total += cur[1] - cur[0]
    return total


oursdir, refdir, out = (pathlib.Path(p) for p in sys.argv[1:4])
tot = 0.0
n = 0
with open(out, 'w') as fh:
    fh.write('identity\tpages\tours_pt\tref_pt\tdelta\n')
    for o in sorted(oursdir.glob('*.pdf')):
        r = refdir / o.name
        if not r.exists():
            continue
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                pages = min(od.page_count, rd.page_count)
                a = sum(cover(od[i]) for i in range(pages))
                b = sum(cover(rd[i]) for i in range(pages))
        except Exception as exc:                      # noqa: BLE001
            print('%s: %s' % (o.name, type(exc).__name__), file=sys.stderr)
            continue
        n += 1
        tot += abs(a - b)
        fh.write('%s\t%d\t%.0f\t%.0f\t%.0f\n' % (o.stem, pages, a, b, a - b))
print('%d documents, sum |ours - reference| = %.0f pt' % (n, tot))
