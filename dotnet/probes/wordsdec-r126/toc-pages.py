#!/usr/bin/env python3
"""Rule cover on contents pages and on ordinary pages, ours against the reference.

    toc-pages.py <ours-dir> <ref-dir> <out.tsv>

Round 124's `toc-page-census.py` re-derived from scratch, with two deliberate changes.

  * It takes its population from the rendered pairs rather than from a declaration census, so it
    cannot inherit that census's `.doc` upper bound (a UTF-16 byte scan for " TOC ").
  * Cover is MERGED per y-band rather than summed, so a rule drawn as several abutting pieces --
    which is exactly what a bridged tab produces -- is not counted several times.  Both legs go
    through the same function, so the comparison is fair either way (C16).

A contents page is identified from the DRAWN text: five or more runs of five or more leader dots
on the page, which is what a reader sees as one and is independent of the markup.  The ordinary
pages of the same documents are the control, and it is the control round 124's seat rests on.
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
ours_more = ref_more = ref_none = 0
ctl_ours_more = ctl_ref_more = 0
tot_o = tot_r = 0.0
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\ttoc_pages\tours_toc_pt\tref_toc_pt\tours_other_median\tref_other_median\n')
    for d in sorted(p for p in oursdir.iterdir() if p.is_dir()):
        pdfs = sorted(d.glob('*.pdf'))
        ref = refdir / (d.name + '.pdf')
        if not pdfs or not ref.exists():
            continue
        try:
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
        except Exception as exc:                      # noqa: BLE001
            print('%s: %s' % (d.name, type(exc).__name__), file=sys.stderr)
            continue
        if not pages:
            continue
        docs += 1
        toc_pages += len(pages)
        tot_o += oc
        tot_r += rc
        if oc - rc > 200:
            ours_more += 1
            if rc == 0:
                ref_none += 1
        if rc - oc > 200:
            ref_more += 1
        mo = statistics.median(oo) if oo else 0.0
        mr = statistics.median(ro) if ro else 0.0
        if mo - mr > 1:
            ctl_ours_more += 1
        if mr - mo > 1:
            ctl_ref_more += 1
        fh.write('%s\t%d\t%.0f\t%.0f\t%.0f\t%.0f\n' % (d.name, len(pages), oc, rc, mo, mr))

print('documents with a drawn contents page: %d, pages %d' % (docs, toc_pages))
print('rule cover on them   ours %.0f pt   reference %.0f pt' % (tot_o, tot_r))
print('ours > reference by 200 pt on them:      %d of %d (reference draws NOTHING: %d)'
      % (ours_more, docs, ref_none))
print('reference > ours by 200 pt on them:      %d of %d' % (ref_more, docs))
print('CONTROL, median of the ORDINARY pages:   ours higher %d of %d, reference higher %d of %d'
      % (ctl_ours_more, docs, ctl_ref_more, docs))
