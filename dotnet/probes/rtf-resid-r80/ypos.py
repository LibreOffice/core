#!/usr/bin/env python3
"""Line origins on one page: y, x, size, first 60 characters."""
import sys, pymupdf
doc = pymupdf.open(sys.argv[1])
p = doc[int(sys.argv[2]) - 1]
rows = []
for b in p.get_text("dict")["blocks"]:
    if b["type"]: continue
    for l in b["lines"]:
        t = "".join(s["text"] for s in l["spans"])
        if not t.strip(): continue
        rows.append((round(l["bbox"][1], 2), round(l["bbox"][0], 2),
                     round(l["spans"][0]["size"], 2), t[:64]))
prev = None
for y, x, s, t in sorted(rows):
    d = "" if prev is None else f"{y-prev:6.2f}"
    print(f"{y:7.2f} {d:>7} x={x:7.2f} sz={s:5.2f} {t}")
    prev = y
