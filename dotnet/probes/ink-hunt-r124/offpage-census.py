#!/usr/bin/env python3
"""How much drawn ink falls OFF the paper, ours against the reference.

    offpage-census.py <rows.tsv> <ours-dir> <ref-dir> <out.tsv>

A mark outside the page box is drawn and then thrown away by the paper's own edge, so it costs no
glyph, no page and often very little ink -- every gate column and the raster diff alike are close
to blind to it. But it is never *intended*: it means a rectangle was composed in the wrong space.

Measured as the area of each drawn item's box that lies outside the media box, in square points,
summed over the document, and counted on BOTH sides so the reference's own overhang is the base
rate rather than an assumption. 26.2.4.2 legitimately draws slightly outside a page -- a bleed, a
full-width rule that starts at a negative x -- so only a document where ours overhangs by orders
more than the reference's is a witness.
"""
import pathlib
import sys

import pymupdf

KINDS = ('fill-path', 'stroke-path', 'fill-image', 'stroke-image', 'fill-text', 'stroke-text',
         'ignore-text')


def overhang(page):
    box = page.rect
    total, worst = 0.0, 0.0
    for b in page.get_bboxlog():
        if b[0] not in KINDS:
            continue
        r = pymupdf.Rect(b[1])
        if r.is_empty or r.is_infinite:
            continue
        r.normalize()
        area = r.width * r.height
        inter = r & box
        inside = 0.0 if inter.is_empty else inter.width * inter.height
        out = area - inside
        if out > 1.0:
            total += out
            worst = max(worst, out)
    return total, worst


rowsfile, oursdir, refdir, out = sys.argv[1:5]
oursdir, refdir = pathlib.Path(oursdir), pathlib.Path(refdir)

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\text\ttrack\tpath\tnpages\tours_off\tref_off\tours_pages\tref_pages'
             '\tworst_page\tworst_item\n')
    for line in pathlib.Path(rowsfile).read_text(encoding='utf-8').splitlines():
        p = line.split('\t')
        if len(p) < 7 or p[6] != 'match':
            continue
        ident = f'{pathlib.PurePosixPath(p[0]).stem}__{p[1]}'
        o, r = oursdir / f'{ident}.pdf', refdir / f'{ident}.pdf'
        if not (o.exists() and r.exists()):
            continue
        oo = rr = 0.0
        op = rp = npages = 0
        worst, worstpg, worstitem = 0.0, 0, 0.0
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                npages = min(od.page_count, rd.page_count)
                for i in range(npages):
                    a, ai = overhang(od[i])
                    b, _ = overhang(rd[i])
                    oo += a
                    rr += b
                    op += 1 if a > 100 else 0
                    rp += 1 if b > 100 else 0
                    if a - b > worst:
                        worst, worstpg, worstitem = a - b, i + 1, ai
        except Exception as exc:                      # noqa: BLE001
            print(f'{ident}: {type(exc).__name__}', file=sys.stderr)
            continue
        fh.write(f'{ident}\t{p[1]}\t{p[0].split("/")[0]}\t{p[0]}\t{npages}\t{oo:.0f}\t{rr:.0f}'
                 f'\t{op}\t{rp}\t{worstpg}\t{worstitem:.0f}\n')
print('done')
