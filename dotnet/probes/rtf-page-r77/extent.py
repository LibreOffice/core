#!/usr/bin/env python3
"""Per-document text extent census over a rendered pair bank."""
import sys, pathlib, pymupdf
ext = sys.argv[1]
OURS = pathlib.Path("/home/user/gate-odf-r76/ours"); REF = pathlib.Path("/home/user/gate-odf-r76/ref")
def ext_of(p):
    d = pymupdf.open(p); tops=[]; bots=[]; hs=set()
    for pg in d:
        w = pg.get_text("words")
        hs.add(round(pg.rect.height,1))
        if w:
            tops.append(min(x[1] for x in w)); bots.append(max(x[3] for x in w))
    if not tops: return None
    return (min(tops), max(bots), d.page_count, sorted(hs)[0])
print("name\tours_top\tref_top\tdtop\tours_bot\tref_bot\tdbot\tours_pg\tref_pg\tph")
for f in sorted(OURS.glob(f"*__{ext}.pdf")):
    g = REF/f.name
    if not g.exists(): continue
    try:
        a = ext_of(f); b = ext_of(g)
    except Exception: continue
    if not a or not b: continue
    print(f"{f.name[:-len('__'+ext+'.pdf')]}\t{a[0]:.2f}\t{b[0]:.2f}\t{a[0]-b[0]:.2f}\t{a[1]:.2f}\t{b[1]:.2f}\t{a[1]-b[1]:.2f}\t{a[2]}\t{b[2]}\t{a[3]}")
