#!/usr/bin/env python3
"""Anchor each page of A into B by locating a distinctive slice of its text.

Both PDFs are reduced to one stream of alphanumeric characters with page boundaries
recorded, so a page of A is placed in B by searching for a slice of its own text.  The
offset between the two page numbers is the pagination drift, and the page at which it
steps is where the extra (or missing) page was made.
"""
import subprocess, sys

def stream(path):
    out = subprocess.run(['pdftotext', path, '-'], capture_output=True).stdout
    txt = out.decode('utf-8', 'replace')
    parts = txt.split('\f')[:-1]
    s, bounds = [], []
    for p in parts:
        s.append(''.join(c for c in p if c.isalnum()))
        bounds.append(sum(len(x) for x in s))
    return ''.join(s), bounds, parts

sa, ba, pa = stream(sys.argv[1])
sb, bb, pb = stream(sys.argv[2])
print(f"A {len(ba)} pages {len(sa)} alnum   B {len(bb)} pages {len(sb)} alnum")

def page_of(bounds, pos):
    for i, b in enumerate(bounds):
        if pos < b:
            return i
    return len(bounds) - 1

prev = None
rows = []
for i in range(len(ba)):
    start = ba[i - 1] if i else 0
    end = ba[i]
    if end - start < 60:
        rows.append((i, None, end - start)); continue
    probe = sa[start + (end - start) // 2: start + (end - start) // 2 + 48]
    j = sb.find(probe)
    rows.append((i, page_of(bb, j) if j >= 0 else None, end - start))

print(" A page  chars   -> B page  offset")
prev_off = 0
for i, j, n in rows:
    off = (j - i) if j is not None else None
    mark = ""
    if off is not None and off != prev_off:
        mark = f"   <== step {prev_off:+d} -> {off:+d}"
        prev_off = off
    print(f" {i+1:>5} {n:>7}   -> {('p'+str(j+1)) if j is not None else '  ?':>6} {('%+d'%off) if off is not None else ' ?':>6}{mark}")
