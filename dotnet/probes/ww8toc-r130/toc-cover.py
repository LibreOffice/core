#!/usr/bin/env python3
"""Rule cover on contents pages, ours against a banked reference, over one sweep directory.

    toc-cover.py <ours-dir> <ref-dir> <out.tsv>

`ours-dir` holds one directory per document (`sweep-doc.py`'s layout); `ref-dir` is flat, one
`<identity>.pdf` per document, which is how a gate bank is laid out.

`cover` and the contents-page test are round 126's `toc-pages.py` verbatim, so the figures are
comparable with `wordsdec-r126/toc-pages-{base,after}.tsv` column for column: a rule is a filled
or stroked rectangle no more than 2 pt tall and at least 4 pt wide, or a horizontal line, MERGED
per y band so that a rule drawn in abutting pieces is not counted several times; a contents page
is one carrying five or more runs of five or more leader dots in its drawn text.
"""
import pathlib
import re
import statistics
import sys

import pymupdf

LEADER = re.compile(r'\.{5,}')
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
docs = toc_pages = 0
tot_o = tot_r = 0.0
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\ttoc_pages\tours_toc_pt\tref_toc_pt\tours_other_median\tref_other_median\n')
    for d in sorted(p for p in oursdir.iterdir() if p.is_dir()):
        pdfs = sorted(d.glob('*.pdf'))
        ref = refdir / (d.name + '.pdf')
        if not pdfs or not ref.exists():
            continue
        with pymupdf.open(pdfs[0]) as od, pymupdf.open(ref) as rd:
            n = min(od.page_count, rd.page_count)
            pages, oc, rc, oo, ro = [], 0.0, 0.0, [], []
            for i in range(n):
                a, b = cover(od[i]), cover(rd[i])
                if len(LEADER.findall(od[i].get_text())) >= 5:
                    pages.append(i + 1)
                    oc += a
                    rc += b
                else:
                    oo.append(a)
                    ro.append(b)
        if not pages:
            continue
        docs += 1
        toc_pages += len(pages)
        tot_o += oc
        tot_r += rc
        mo = statistics.median(oo) if oo else 0.0
        mr = statistics.median(ro) if ro else 0.0
        fh.write('%s\t%d\t%.0f\t%.0f\t%.0f\t%.0f\n' % (d.name, len(pages), oc, rc, mo, mr))

print('documents with a drawn contents page: %d, pages %d' % (docs, toc_pages))
print('rule cover on them   ours %.0f pt   reference %.0f pt' % (tot_o, tot_r))
