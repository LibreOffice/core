#!/usr/bin/env python3
"""Merged horizontal-rule cover per page, and the distance from the reference's.

    cover.py <ours-dir> <ref-dir> <out.tsv> [--dirs] [only.txt]

Cover rather than a count, because the two sides consolidate a grid line differently and an object
count is not comparable (C16). A rule is merged into its y-band before it is summed, so drawing one
stroke where the reference draws three is not a difference here and drawing a HOLE is.
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
                if r.height <= 2.0 and r.width >= 4.0 and k in ('f', 'fs', 's'):
                    bands.setdefault(round((r.y0 + r.y1) / 2 / TOL_Y), []).append((r.x0, r.x1))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 2.0 and abs(a.x - b.x) >= 4.0:
                    bands.setdefault(round((a.y + b.y) / 2 / TOL_Y), []).append(
                        (min(a.x, b.x), max(a.x, b.x)))
    total = 0.0
    for spans in bands.values():
        spans.sort()
        end = None
        for a, b in spans:
            if end is None or a > end:
                total += b - a
                end = b
            elif b > end:
                total += b - end
                end = b
    return total


oursdir, refdir, out = (pathlib.Path(p) for p in sys.argv[1:4])
dirs = '--dirs' in sys.argv[4:]
only = None
for arg in sys.argv[4:]:
    if arg != '--dirs':
        only = set(pathlib.Path(arg).read_text().splitlines())

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\tpages\tours_pt\tref_pt\tabs_distance_pt\n')
    for r in sorted(refdir.glob('*.pdf')):
        if only is not None and r.stem not in only:
            continue
        if dirs:
            d = oursdir / r.stem
            pdfs = sorted(d.glob('*.pdf')) if d.is_dir() else []
            if not pdfs:
                continue
            o = pdfs[0]
        else:
            o = oursdir / r.name
            if not o.exists():
                continue
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                n = min(od.page_count, rd.page_count)
                ours = ref = dist = 0.0
                for i in range(n):
                    a, b = cover(od[i]), cover(rd[i])
                    ours += a
                    ref += b
                    dist += abs(a - b)
        except Exception as exc:                       # noqa: BLE001
            print(f'{r.stem}: {type(exc).__name__}', file=sys.stderr)
            continue
        fh.write(f'{r.stem}\t{n}\t{ours:.0f}\t{ref:.0f}\t{dist:.0f}\n')
print('done')
