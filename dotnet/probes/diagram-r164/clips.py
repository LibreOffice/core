#!/usr/bin/env python3
"""The clip paths the reference puts in the page, and ours (we emit none).

get_drawings(extended=True) reports the clip stack as 'clip' items. Matching a clip's
rectangle against the shape whose text it cuts tells the patch which rectangle to use.
"""
import pathlib, sys
import pymupdf
import census as C

here = pathlib.Path(__file__).parent


def clips(pdf):
    d = pymupdf.open(pdf)
    out = []
    for p in d:
        for it in p.get_drawings(extended=True):
            if it['type'] == 'clip':
                r = it.get('scissor') or it.get('rect')
                out.append((round(r.x0, 1), round(r.y0, 1), round(r.x1, 1), round(r.y1, 1)))
    d.close()
    return out


rows = [l.split('\t') for l in (here / 'docs.tsv').read_text().split('\n') if l.strip()]
for stem, _ in rows:
    o = clips(C.only_pdf(str(here / 'ours' / stem)))
    r = clips(C.only_pdf(str(here / 'ref' / stem)))
    ro = sorted(set(r) - set(o))
    print('### %-52s ref clips=%d (distinct %d)   our clips=%d'
          % (stem[:52], len(r), len(set(r)), len(o)))
    for c in ro[:6]:
        print('      ref-only clip rect', c)
