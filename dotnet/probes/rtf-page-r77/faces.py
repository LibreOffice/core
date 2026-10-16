#!/usr/bin/env python3
import sys, pathlib, pymupdf, collections
OURS=pathlib.Path("/home/user/gate-odf-r76/ours"); REF=pathlib.Path("/home/user/gate-odf-r76/ref")
ext=sys.argv[1]
def faces(p):
    d=pymupdf.open(p); s=set()
    for pg in d:
        for f in pg.get_fonts(full=False):
            n=f[3]
            s.add(n.split('+')[-1])
    return s
print("name\tref_only\tours_only")
for f in sorted(OURS.glob(f"*__{ext}.pdf")):
    g=REF/f.name
    if not g.exists(): continue
    try: a=faces(f); b=faces(g)
    except Exception: continue
    print(f"{f.name[:-len('__'+ext+'.pdf')]}\t{','.join(sorted(b-a))}\t{','.join(sorted(a-b))}")
