#!/usr/bin/env python3
"""Confine a change over one track: which renderings move, and whether any gate column does."""
import hashlib, sys, re
from pathlib import Path
import pymupdf

A = Path(sys.argv[1]); B = Path(sys.argv[2])
alnum = re.compile(r'[^0-9A-Za-zÀ-￿]')

def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()

def counts(p):
    d = pymupdf.open(p)
    n = d.page_count
    g = 0
    for i in range(n):
        t = d[i].get_text()
        g += sum(1 for c in t if c.isalnum())
    d.close()
    return n, g

moved = []
names = sorted(x.name for x in A.glob("*.pdf"))
missing = 0
for name in names:
    a, b = A / name, B / name
    if not b.exists(): missing += 1; continue
    if digest(a) == digest(b): continue
    pa, ga = counts(a); pb, gb = counts(b)
    moved.append((name, pa, pb, ga, gb))

print(f"documents {len(names)}  missing-in-after {missing}  bytes-moved {len(moved)}")
pagemoves = [m for m in moved if m[1] != m[2]]
glyphmoves = [m for m in moved if m[3] != m[4]]
print(f"  page count moved   {len(pagemoves)}")
print(f"  glyph count moved  {len(glyphmoves)}")
for m in pagemoves: print("   PAGE", m)
for m in glyphmoves: print("   GLYPH", m)
with open(sys.argv[3], "w") as f:
    f.write("doc\tpages_base\tpages_after\tglyphs_base\tglyphs_after\n")
    for m in moved: f.write("\t".join(str(x) for x in m) + "\n")
