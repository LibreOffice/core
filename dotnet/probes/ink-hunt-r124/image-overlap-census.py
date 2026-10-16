#!/usr/bin/env python3
"""Pairs of drawn images that overlap, ours against the reference.

    image-overlap-census.py <rows.tsv> <ours-dir> <ref-dir> <out.tsv> [ext-filter]

Two inline pictures that should have taken a line each, drawn on one line, land on top of one
another. This counts that shape directly out of both PDFs' image placements, and it carries its
own base rate: the reference's overlapping pairs are counted the same way on the same pages, and
a document only reads as a witness where ours overlap and the reference's do not.

Overlap is at least half of the SMALLER rectangle's area, so a picture deliberately laid over
another -- a watermark, a logo on a banner -- is counted on both sides alike and cancels.
"""
import pathlib
import sys

import pymupdf

SHARE = 0.5


def boxes(page):
    out = []
    for b in page.get_bboxlog():
        if b[0] in ('fill-image', 'stroke-image'):
            r = pymupdf.Rect(b[1])
            if r.width > 4 and r.height > 4:
                out.append(r)
    return out


def overlaps(rs):
    n = 0
    for i in range(len(rs)):
        for j in range(i + 1, len(rs)):
            a, b = rs[i], rs[j]
            inter = a & b
            if inter.is_empty:
                continue
            area = inter.width * inter.height
            small = min(a.width * a.height, b.width * b.height)
            if small > 0 and area / small >= SHARE:
                n += 1
    return n


rowsfile, oursdir, refdir, out = sys.argv[1:5]
extf = sys.argv[5].split(',') if len(sys.argv) > 5 else None
oursdir, refdir = pathlib.Path(oursdir), pathlib.Path(refdir)

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\text\tpath\tnpages\tours_overlaps\tref_overlaps\tours_pages\tref_pages'
             '\tworst_page\n')
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
        worst, worstpg = 0, 0
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                npages = min(od.page_count, rd.page_count)
                for i in range(npages):
                    a, b = overlaps(boxes(od[i])), overlaps(boxes(rd[i]))
                    on += a
                    rn += b
                    op += 1 if a else 0
                    rp += 1 if b else 0
                    if a - b > worst:
                        worst, worstpg = a - b, i + 1
        except Exception as exc:                      # noqa: BLE001
            print(f'{ident}: {type(exc).__name__}', file=sys.stderr)
            continue
        if on or rn:
            fh.write(f'{ident}\t{p[1]}\t{p[0]}\t{npages}\t{on}\t{rn}\t{op}\t{rp}\t{worstpg}\n')
print('done')
