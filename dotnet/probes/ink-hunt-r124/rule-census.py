#!/usr/bin/env python3
"""Count the thin horizontal rules each side actually DRAWS, per page.

    rule-census.py <rows.tsv> <ours-dir> <ref-dir> <out.tsv> [ext-filter]

Not a declaration census: this reads both PDFs' own drawing operators, so it measures what
reached the page rather than what the file states.

**C16 is the whole reason this counts both shapes.** This tree emits a text rule as a FILL and
26.2.4.2 emits one as a STROKE, so a census that looks for strokes scores every one of ours as
absent and a census that looks for fills scores every one of the reference's as absent. A rule
here is *either* a filled rectangle thinner than 2 pt and wider than 8, *or* a stroked segment
of the same shape, on either side.

**And C14's half of the same trap**: a hairline is written `0 w` and has no measurable height,
so a zero-height stroked segment counts as a rule rather than as nothing.

The column that decides is `covered_pt` -- the total horizontal LENGTH of rule on the page --
not the object count. One hyperlink cell is one stroke at the reference and as many fills as it
has rich segments here (C16 again), so counts differ by construction where lengths agree.
"""
import pathlib
import sys

import pymupdf

MAX_THICK = 2.0
MIN_WIDE = 8.0


def rules(page):
    out = []
    for d in page.get_drawings():
        kind = d.get('type', '')
        for item in d['items']:
            if item[0] == 're':
                r = item[1]
                if r.height <= MAX_THICK and r.width >= MIN_WIDE and kind in ('f', 'fs'):
                    out.append((r.x0, r.x1, r.y0))
            elif item[0] == 'l' and kind in ('s', 'fs'):
                a, b = item[1], item[2]
                if abs(a.y - b.y) <= MAX_THICK and abs(a.x - b.x) >= MIN_WIDE:
                    out.append((min(a.x, b.x), max(a.x, b.x), a.y))
        # a stroked rectangle degenerate in height is a rule too
        if kind in ('s', 'fs'):
            for item in d['items']:
                if item[0] == 're':
                    r = item[1]
                    if r.height <= MAX_THICK and r.width >= MIN_WIDE:
                        out.append((r.x0, r.x1, r.y0))
    return out


def summarise(page):
    rs = rules(page)
    return len(rs), sum(b - a for a, b, _ in rs)


rowsfile, oursdir, refdir, out = sys.argv[1:5]
extf = sys.argv[5].split(',') if len(sys.argv) > 5 else None
oursdir, refdir = pathlib.Path(oursdir), pathlib.Path(refdir)

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\text\tpath\tnpages\tours_rules\tref_rules\tours_pt\tref_pt'
             '\tpages_ours_excess\tworst_page\tworst_excess\n')
    for line in pathlib.Path(rowsfile).read_text(encoding='utf-8').splitlines():
        p = line.split('\t')
        if len(p) < 7 or p[6] != 'match':
            continue
        if extf and p[1] not in extf:
            continue
        ident = f'{pathlib.PurePosixPath(p[0]).stem}__{p[1]}'
        o, r = oursdir / f'{ident}.pdf', refdir / f'{ident}.pdf'
        if not (o.exists() and r.exists()):
            continue
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                n = min(od.page_count, rd.page_count)
                on = rn = 0
                opt = rpt = 0.0
                excess_pages, worst, worstpg = 0, 0.0, 0
                for i in range(n):
                    a, al = summarise(od[i])
                    b, bl = summarise(rd[i])
                    on += a
                    rn += b
                    opt += al
                    rpt += bl
                    if al - bl > 50:
                        excess_pages += 1
                    if al - bl > worst:
                        worst, worstpg = al - bl, i + 1
        except Exception as exc:                      # noqa: BLE001
            print(f'{ident}: {type(exc).__name__}', file=sys.stderr)
            continue
        fh.write(f'{ident}\t{p[1]}\t{p[0]}\t{n}\t{on}\t{rn}\t{opt:.0f}\t{rpt:.0f}'
                 f'\t{excess_pages}\t{worstpg}\t{worst:.0f}\n')
print('done')
