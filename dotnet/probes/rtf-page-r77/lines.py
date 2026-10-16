#!/usr/bin/env python3
"""Total text-line count and mean line pitch, ours vs ref, per document."""
import sys, pathlib, pymupdf
OURS=pathlib.Path("/home/user/gate-odf-r76/ours"); REF=pathlib.Path("/home/user/gate-odf-r76/ref")
ext=sys.argv[1]
def stat(p):
    d=pymupdf.open(p); n=0; pitches=[]
    for pg in d:
        ys=sorted({round(w[1],1) for w in pg.get_text("words")})
        prev=None; c=0
        for y in ys:
            if prev is None or y-prev>1.5:
                if prev is not None: pitches.append(y-prev)
                c+=1; prev=y
        n+=c
    pitches.sort()
    med = pitches[len(pitches)//2] if pitches else 0
    return n, med, d.page_count
print("name\tours_lines\tref_lines\tdlines\tours_pitch\tref_pitch\tours_pg\tref_pg")
for f in sorted(OURS.glob(f"*__{ext}.pdf")):
    g=REF/f.name
    if not g.exists(): continue
    try: a=stat(f); b=stat(g)
    except Exception: continue
    print(f"{f.name[:-len('__'+ext+'.pdf')]}\t{a[0]}\t{b[0]}\t{a[0]-b[0]}\t{a[1]:.2f}\t{b[1]:.2f}\t{a[2]}\t{b[2]}")
