#!/usr/bin/env python3
"""Census: for every rendered pair of one extension, report page counts and whether the last page is textless."""
import sys, pathlib, pymupdf

ext = sys.argv[1] if len(sys.argv) > 1 else "rtf"
OURS = pathlib.Path("/home/user/gate-odf-r76/ours")
REF  = pathlib.Path("/home/user/gate-odf-r76/ref")

def info(p):
    d = pymupdf.open(p)
    n = d.page_count
    empt = 0
    for pg in reversed(range(n)):
        if d[pg].get_text("words"):
            break
        empt += 1
    return n, empt

print("ours_pages\tref_pages\tours_trailing_empty\tref_trailing_empty\tname")
for f in sorted(OURS.glob(f"*__{ext}.pdf")):
    g = REF / f.name
    if not g.exists():
        continue
    try:
        a, ae = info(f); b, be = info(g)
    except Exception as e:
        print(f"ERR\t{f.name}\t{e}"); continue
    print(f"{a}\t{b}\t{ae}\t{be}\t{f.name[:-len('__'+ext+'.pdf')]}")
