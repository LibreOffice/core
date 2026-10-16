#!/usr/bin/env python3
"""`ink-hunt-r124/gap-census.py`'s two directions over the whole words track, both legs.

    gap-track.py <ours-dir> <ref-dir> <out.tsv> [--dirs]

Same thresholds and the same shape as the census O82 is stated in -- TOL_Y 1.2, MIN_GAP 8,
COVER 0.8 -- so the totals here are comparable with round 126's `gap-after.tsv`. `--dirs` reads
one directory per document instead of one PDF per document.
"""
import pathlib
import sys

import pymupdf

TOL_Y, MIN_GAP, COVER = 1.2, 8.0, 0.8


def rules(page):
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 2.0 and r.width >= 4.0 and k in ('f', 'fs', 's'):
                    out.append((r.x0, r.x1, (r.y0 + r.y1) / 2))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 2.0 and abs(a.x - b.x) >= 4.0:
                    out.append((min(a.x, b.x), max(a.x, b.x), (a.y + b.y) / 2))
    return out


def bands(rs):
    out = {}
    for x0, x1, y in rs:
        out.setdefault(round(y / TOL_Y), []).append((x0, x1))
    for k, v in out.items():
        v.sort()
        merged = []
        for a, b in v:
            if merged and a <= merged[-1][1] + 0.5:
                merged[-1][1] = max(merged[-1][1], b)
            else:
                merged.append([a, b])
        out[k] = merged
    return out


def bridges(mine, theirs):
    total, count = 0.0, 0
    for key, spans in mine.items():
        other = theirs.get(key) or theirs.get(key - 1) or theirs.get(key + 1)
        if not other or len(spans) < 2:
            continue
        for (a1, b1), (a2, _) in zip(spans, spans[1:]):
            gap = a2 - b1
            if gap < MIN_GAP:
                continue
            cov = sum(max(0.0, min(b1 + gap, y1) - max(b1, y0)) for y0, y1 in other)
            if cov >= COVER * gap:
                total += gap
                count += 1
    return count, total


oursdir, refdir, out = (pathlib.Path(p) for p in sys.argv[1:4])
dirs = '--dirs' in sys.argv[4:]

items = []
for r in sorted(refdir.glob('*.pdf')):
    if dirs:
        d = oursdir / r.stem
        pdfs = sorted(d.glob('*.pdf')) if d.is_dir() else []
        if pdfs:
            items.append((r.stem, pdfs[0], r))
    else:
        o = oursdir / r.name
        if o.exists():
            items.append((r.stem, o, r))

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\tnpages\tref_bridges\tref_bridge_pt\tref_bridge_pages'
             '\tours_bridges\tours_bridge_pt\tours_bridge_pages\tworst_page\n')
    for ident, o, r in items:
        rb = rp = ob = op = 0
        rbl = obl = 0.0
        worst, worstpg = 0.0, 0
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                n = min(od.page_count, rd.page_count)
                for i in range(n):
                    mo, mr = bands(rules(od[i])), bands(rules(rd[i]))
                    c, t = bridges(mo, mr)
                    d2, u = bridges(mr, mo)
                    rb += c; rbl += t; ob += d2; obl += u
                    rp += 1 if c else 0
                    op += 1 if d2 else 0
                    if t > worst:
                        worst, worstpg = t, i + 1
        except Exception as exc:                       # noqa: BLE001
            print(f'{ident}: {type(exc).__name__}', file=sys.stderr)
            continue
        if rb or ob:
            fh.write(f'{ident}\t{n}\t{rb}\t{rbl:.0f}\t{rp}\t{ob}\t{obl:.0f}\t{op}\t{worstpg}\n')
print('done')
