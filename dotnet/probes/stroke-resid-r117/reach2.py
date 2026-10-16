#!/usr/bin/env python3
"""Confine a change over one track: which renderings move, and whether any gate column does.

Round 115's `reach.py` with the two columns the gate actually decides on named explicitly:
page count, and alphanumeric characters (the gate's column 9).  Columns 4 and 8 are token
counts and decide nothing, so they are not computed here.
"""
import hashlib, sys
from pathlib import Path
import pymupdf

A = Path(sys.argv[1]); B = Path(sys.argv[2])


def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def counts(p):
    d = pymupdf.open(p)
    n = d.page_count
    g = 0
    for i in range(n):
        g += sum(1 for c in d[i].get_text() if c.isalnum())
    d.close()
    return n, g


moved = []
names = sorted(x.name for x in A.glob("*.pdf"))
missing = 0
for name in names:
    a, b = A / name, B / name
    if not b.exists():
        missing += 1
        continue
    if digest(a) == digest(b):
        continue
    pa, ga = counts(a)
    pb, gb = counts(b)
    moved.append((name, pa, pb, ga, gb))

print(f"documents {len(names)}  missing-in-after {missing}  bytes-moved {len(moved)}")
pagemoves = [m for m in moved if m[1] != m[2]]
glyphmoves = [m for m in moved if m[3] != m[4]]
print(f"  page count moved   {len(pagemoves)}")
print(f"  alnum count moved  {len(glyphmoves)}")
for m in pagemoves:
    print("   PAGE", m)
for m in glyphmoves:
    print("   ALNUM", m)
with open(sys.argv[3], "w") as f:
    f.write("doc\tpages_base\tpages_after\talnum_base\talnum_after\n")
    for m in moved:
        f.write("\t".join(str(x) for x in m) + "\n")
