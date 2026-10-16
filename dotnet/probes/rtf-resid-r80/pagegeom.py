#!/usr/bin/env python3
"""Per-page text geometry of a PDF: first/last baseline, line count, size histogram.

Usage: pagegeom.py <pdf> [firstpage] [lastpage]
"""
import sys, pymupdf, collections

doc = pymupdf.open(sys.argv[1])
lo = int(sys.argv[2]) if len(sys.argv) > 2 else 1
hi = int(sys.argv[3]) if len(sys.argv) > 3 else min(len(doc), lo + 5)
print(f"{len(doc)} pages, mediabox {doc[0].rect}")
for i in range(lo - 1, min(hi, len(doc))):
    p = doc[i]
    d = p.get_text("dict")
    lines = [l for b in d["blocks"] if b["type"] == 0 for l in b["lines"]]
    ys = sorted(l["bbox"][1] for l in lines)
    xs = sorted(l["bbox"][0] for l in lines)
    sizes = collections.Counter(round(s["size"], 2) for l in lines for s in l["spans"])
    print(f"p{i+1}: lines={len(lines):4d} ytop={ys[0]:7.2f} ybot={ys[-1]:7.2f} "
          f"xmin={xs[0]:7.2f} xmax={max(l['bbox'][2] for l in lines):7.2f} "
          f"sizes={sizes.most_common(4)}" if lines else f"p{i+1}: empty")
