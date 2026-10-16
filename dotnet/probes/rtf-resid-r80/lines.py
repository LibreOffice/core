#!/usr/bin/env python3
"""Whole-document line and character totals, and the lines-per-page distribution."""
import sys, pymupdf, statistics
for path in sys.argv[1:]:
    doc = pymupdf.open(path)
    per = []
    chars = 0
    for p in doc:
        d = p.get_text("dict")
        ls = [l for b in d["blocks"] if b["type"] == 0 for l in b["lines"]]
        per.append(len(ls))
        chars += sum(len(s["text"]) for l in ls for s in l["spans"])
    print(f"{path.split('/')[-1]:34s} pages={len(doc):4d} lines={sum(per):6d} "
          f"chars={chars:8d} lines/page mean={statistics.mean(per):6.2f} "
          f"median={statistics.median(per):5.1f} max={max(per)}")
