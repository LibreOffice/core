#!/usr/bin/env python3
"""Rule cover on the pages that ARE a table of contents, ours against the reference.

    toc-page-census.py <toc-census.tsv> <rows.tsv> <ours-dir> <ref-dir> <out.tsv>

A TOC page is identified from the drawn text rather than from the markup: a page carrying at
least five runs of five or more leader dots. That is what a reader sees as a contents page, and
it does not depend on the field's own spelling.

The comparison is horizontal rule COVER on those pages only, so the header-band defect the same
documents carry (an underline stopping at a tab, which costs every page alike) cannot leak into
it: it is subtracted as the median cover of the document's NON-TOC pages.
"""
import pathlib
import re
import statistics
import sys

import pymupdf

LEADER = re.compile(r'\.{5,}')


def cover(page):
    total = 0.0
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 2.0 and r.width >= 4.0 and k in ('f', 'fs', 's'):
                    total += r.width
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 2.0 and abs(a.x - b.x) >= 4.0:
                    total += abs(a.x - b.x)
    return total


toccensus, rowsfile, oursdir, refdir, out = sys.argv[1:6]
oursdir, refdir = pathlib.Path(oursdir), pathlib.Path(refdir)
want = {}
for line in pathlib.Path(toccensus).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    want[p[0]] = (int(p[2]), int(p[3]))

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\text\tpath\ttoc_fields\thyp_runs\ttoc_pages\tours_toc_pt\tref_toc_pt'
             '\tours_other_median\tref_other_median\tfirst_toc_page\n')
    for line in pathlib.Path(rowsfile).read_text(encoding='utf-8').splitlines():
        p = line.split('\t')
        if len(p) < 7 or p[6] != 'match' or p[0] not in want:
            continue
        ident = f'{pathlib.PurePosixPath(p[0]).stem}__{p[1]}'
        o, r = oursdir / f'{ident}.pdf', refdir / f'{ident}.pdf'
        if not (o.exists() and r.exists()):
            continue
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                n = min(od.page_count, rd.page_count)
                toc_pages, oc, rc, oo, ro = [], 0.0, 0.0, [], []
                for i in range(n):
                    t = od[i].get_text()
                    a, b = cover(od[i]), cover(rd[i])
                    if len(LEADER.findall(t)) >= 5:
                        toc_pages.append(i + 1)
                        oc += a
                        rc += b
                    else:
                        oo.append(a)
                        ro.append(b)
        except Exception as exc:                      # noqa: BLE001
            print(f'{ident}: {type(exc).__name__}', file=sys.stderr)
            continue
        if not toc_pages:
            continue
        fh.write(f'{ident}\t{p[1]}\t{p[0]}\t{want[p[0]][0]}\t{want[p[0]][1]}\t{len(toc_pages)}'
                 f'\t{oc:.0f}\t{rc:.0f}\t{statistics.median(oo) if oo else 0:.0f}'
                 f'\t{statistics.median(ro) if ro else 0:.0f}\t{toc_pages[0]}\n')
print('done')
