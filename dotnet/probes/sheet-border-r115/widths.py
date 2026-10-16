#!/usr/bin/env python3
"""Stroke-width histogram of a PDF, over up to N sampled pages."""
import sys, collections
import pymupdf

def widths(path, maxpages=8):
    doc = pymupdf.open(path)
    n = doc.page_count
    if n <= maxpages:
        pages = range(n)
    else:
        step = n / maxpages
        pages = sorted({int(i*step) for i in range(maxpages)})
    hist = collections.Counter()
    for p in pages:
        for d in doc[p].get_drawings():
            if d["type"] in ("s", "fs") and d.get("width") is not None:
                hist[round(d["width"], 4)] += 1
    doc.close()
    return n, hist

for path in sys.argv[1:]:
    n, h = widths(path)
    tot = sum(h.values())
    print(f"{path}  pages={n} strokes={tot}")
    for w, c in sorted(h.items()):
        print(f"   {w:8.4f}  x{c}")
