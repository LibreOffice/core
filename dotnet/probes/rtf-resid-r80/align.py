#!/usr/bin/env python3
"""Where the same text lands in two renderings.

Takes the reference's page-N first prose line and reports which of our pages holds it,
for a sample of pages -- so the drift can be read as a curve rather than as a total.
Usage: align.py <ours.pdf> <ref.pdf> [step]
"""
import sys, pymupdf, re

ours, ref = pymupdf.open(sys.argv[1]), pymupdf.open(sys.argv[2])
step = int(sys.argv[3]) if len(sys.argv) > 3 else 25

def norm(t):
    return re.sub(r"[^a-z0-9]", "", t.lower())

otext = [norm(p.get_text()) for p in ours]
print(f"ours {len(ours)} pages, ref {len(ref)} pages")
print("refpage\tourpage\tdrift\tkey")
for i in range(0, len(ref), step):
    lines = [l for l in ref[i].get_text().splitlines() if len(norm(l)) > 40]
    if not lines:
        continue
    key = norm(lines[len(lines) // 2])[:60]
    hit = next((j + 1 for j, t in enumerate(otext) if key in t), None)
    print(f"{i+1}\t{hit}\t{'' if hit is None else hit-(i+1)}\t{key[:40]}")
