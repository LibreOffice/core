#!/usr/bin/env python3
"""Per-page paper size, ours against the banked reference, over the whole passing set.

    page-size-census.py <rows.tsv> <ours-dir> <ref-dir> <out.tsv>

This is not a raster measurement and needs no rendering: it reads each page's own box out of
the two PDFs. It exists because `pdf-image-diff.py` REFUSES a page whose two renderings differ
in size beyond a rounding step -- it prints `page size differs` and counts the page as major --
so a page of this class is silently absent from any ink table built by parsing that tool's
numeric rows, including this round's. The gate cannot see one either: a page that is the wrong
shape is still a page, and its text still extracts.
"""
import pathlib
import sys

import pymupdf

rowsfile, oursdir, refdir, out = sys.argv[1:5]
oursdir, refdir = pathlib.Path(oursdir), pathlib.Path(refdir)

rows = []
for line in pathlib.Path(rowsfile).read_text(encoding='utf-8').splitlines():
    p = line.split('\t')
    if len(p) >= 7 and p[6] == 'match':
        rows.append((f'{pathlib.PurePosixPath(p[0]).stem}__{p[1]}', p[0], p[1], p[0].split('/')[0]))

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\text\ttrack\tpath\tnpages\tdiffering\tworst_pt\tfirst_page'
             '\tours_wxh\tref_wxh\torientation_flips\n')
    for ident, rel, ext, track in rows:
        o, r = oursdir / f'{ident}.pdf', refdir / f'{ident}.pdf'
        if not (o.exists() and r.exists()):
            continue
        with pymupdf.open(o) as od, pymupdf.open(r) as rd:
            n = min(od.page_count, rd.page_count)
            diff, worst, first, ow, rw, flips = 0, 0.0, '', '', '', 0
            for i in range(n):
                a, b = od[i].rect, rd[i].rect
                d = max(abs(a.width - b.width), abs(a.height - b.height))
                if d > 0.5:
                    diff += 1
                    if (a.width > a.height) != (b.width > b.height):
                        flips += 1
                    if d > worst:
                        worst, first = d, str(i + 1)
                        ow = f'{a.width:.1f}x{a.height:.1f}'
                        rw = f'{b.width:.1f}x{b.height:.1f}'
            fh.write(f'{ident}\t{ext}\t{track}\t{rel}\t{n}\t{diff}\t{worst:.2f}\t{first}'
                     f'\t{ow}\t{rw}\t{flips}\n')
print('done')
