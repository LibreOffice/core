#!/usr/bin/env python3
"""Rules one side draws on a baseline the other side leaves entirely empty.

    solo-census.py <rows.tsv> <ours-dir> <ref-dir> <out.tsv> [ext-filter]

`gap-census.py` measures an underline that STOPS EARLY. This measures one that should not be
there at all, or is missing altogether: a y-band carrying rule in one rendering and no rule
within 1.2 pt in the other.

Both shapes are counted on both sides for the same C16 reason as the other two censuses -- this
tree fills a text rule and 26.2.4.2 strokes one -- and the figure quoted is horizontal COVER in
points, never the object count.
"""
import pathlib
import sys

import pymupdf

TOL_Y = 1.2


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


def solo(mine, theirs):
    keys = set()
    for _, _, y in theirs:
        k = round(y / TOL_Y)
        keys.update((k - 1, k, k + 1))
    n, length = 0, 0.0
    for x0, x1, y in mine:
        if round(y / TOL_Y) not in keys:
            n += 1
            length += x1 - x0
    return n, length


rowsfile, oursdir, refdir, out = sys.argv[1:5]
extf = sys.argv[5].split(',') if len(sys.argv) > 5 else None
oursdir, refdir = pathlib.Path(oursdir), pathlib.Path(refdir)

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\text\tpath\tnpages\tours_solo\tours_solo_pt\tours_solo_pages'
             '\tref_solo\tref_solo_pt\tref_solo_pages\tworst_page\n')
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
        on = rn = op = rp = npages = 0
        ol = rl = 0.0
        worst, worstpg = 0.0, 0
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                npages = min(od.page_count, rd.page_count)
                for i in range(npages):
                    mo, mr = rules(od[i]), rules(rd[i])
                    a, al = solo(mo, mr)
                    b, bl = solo(mr, mo)
                    on += a
                    ol += al
                    rn += b
                    rl += bl
                    op += 1 if al > 20 else 0
                    rp += 1 if bl > 20 else 0
                    if al > worst:
                        worst, worstpg = al, i + 1
        except Exception as exc:                      # noqa: BLE001
            print(f'{ident}: {type(exc).__name__}: {exc}', file=sys.stderr)
            continue
        if on or rn:
            fh.write(f'{ident}\t{p[1]}\t{p[0]}\t{npages}\t{on}\t{ol:.0f}\t{op}'
                     f'\t{rn}\t{rl:.0f}\t{rp}\t{worstpg}\n')
print('done')
