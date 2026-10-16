#!/usr/bin/env python3
"""Pairs of images sharing a BASELINE, ours against the reference.

    baseline-pair-census.py <rows.tsv> <ours-dir> <ref-dir> <out.tsv> [ext-filter]

Two as-character pictures kept on one line sit on one baseline, so their drawn boxes share a
bottom edge to within a rounding step and overlap horizontally. That is a narrower and more
specific shape than "two images overlap", which a watermark or a layered logo also makes, and it
is the one an unbroken line produces.

Counted on both sides, so the reference's own baseline-sharing pairs -- a row of small inline
icons, which is legitimate and common -- are the base rate rather than an assumption. Only a pair
whose two boxes overlap by at least half of the smaller is counted, which a row of icons side by
side never does.
"""
import pathlib
import sys

import pymupdf


def boxes(page):
    out = []
    for b in page.get_bboxlog():
        if b[0] in ('fill-image', 'stroke-image'):
            r = pymupdf.Rect(b[1])
            if r.width > 20 and r.height > 20:
                out.append(r)
    return out


def pairs(rs):
    n = 0
    for i in range(len(rs)):
        for j in range(i + 1, len(rs)):
            a, b = rs[i], rs[j]
            if abs(a.y1 - b.y1) > 0.5:
                continue
            inter = a & b
            if inter.is_empty:
                continue
            area = inter.width * inter.height
            small = min(a.width * a.height, b.width * b.height)
            if small > 0 and area / small >= 0.5:
                n += 1
    return n


rowsfile, oursdir, refdir, out = sys.argv[1:5]
extf = sys.argv[5].split(',') if len(sys.argv) > 5 else None
oursdir, refdir = pathlib.Path(oursdir), pathlib.Path(refdir)

o_docs = r_docs = 0
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\text\tpath\tnpages\tours_pairs\tref_pairs\tworst_page\n')
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
        on = rn = npages = 0
        worst, worstpg = 0, 0
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                npages = min(od.page_count, rd.page_count)
                for i in range(npages):
                    a, b = pairs(boxes(od[i])), pairs(boxes(rd[i]))
                    on += a
                    rn += b
                    if a - b > worst:
                        worst, worstpg = a - b, i + 1
        except Exception as exc:                      # noqa: BLE001
            print(f'{ident}: {type(exc).__name__}', file=sys.stderr)
            continue
        if on or rn:
            fh.write(f'{ident}\t{p[1]}\t{p[0]}\t{npages}\t{on}\t{rn}\t{worstpg}\n')
            o_docs += 1 if on > rn else 0
            r_docs += 1 if rn > on else 0
print(f'ours more baseline-sharing image pairs in {o_docs} documents; the reference in {r_docs}')
