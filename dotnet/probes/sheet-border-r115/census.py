#!/usr/bin/env python3
"""Like-for-like stroke-width census: ours against a reference bank.

Restricted to documents where both sides emit >=10 stroked items and their counts agree
within 25 %, which is the guard that keeps a document where one side fills what the other
strokes out of the comparison. Eight pages sampled per document, as in round 113.
"""
import sys, collections
from pathlib import Path
import pymupdf

def sample(path, maxpages=8):
    doc = pymupdf.open(path)
    n = doc.page_count
    pages = range(n) if n <= maxpages else sorted(
        {int(i * n / maxpages) for i in range(maxpages)})
    widths = []
    for p in pages:
        for d in doc[p].get_drawings():
            if d["type"] in ("s", "fs") and d.get("width") is not None:
                widths.append(d["width"])
    doc.close()
    return widths

OURS = Path(sys.argv[1]); REF = Path(sys.argv[2]); OUT = Path(sys.argv[3])
rows = []
for ourpdf in sorted(OURS.glob("*.pdf")):
    refpdf = REF / ourpdf.name
    if not refpdf.exists(): continue
    try:
        a = sample(ourpdf); b = sample(refpdf)
    except Exception as e:
        print("SKIP", ourpdf.name, e); continue
    if len(a) < 10 or len(b) < 10: continue
    if abs(len(a) - len(b)) > 0.25 * max(len(a), len(b)): continue
    ma = sum(a) / len(a); mb = sum(b) / len(b)
    rows.append((ourpdf.name, len(a), len(b), ma, mb, ma / mb if mb else 0.0))

with OUT.open("w") as f:
    f.write("doc\tours_items\tref_items\tours_meanw\tref_meanw\twidth_ratio\n")
    for r in rows:
        f.write(f"{r[0]}\t{r[1]}\t{r[2]}\t{r[3]:.3f}\t{r[4]:.3f}\t{r[5]:.3f}\n")

ratios = sorted(r[5] for r in rows)
def band(lo, hi): return sum(1 for x in ratios if lo <= x < hi)
n = len(ratios)
print(f"comparable {n}  median {ratios[n//2]:.3f}" if n else "comparable 0")
print(f"  ours lighter by >10%: {sum(1 for x in ratios if x < 0.9)}")
print(f"  within 10%:           {sum(1 for x in ratios if 0.9 <= x <= 1.1)}")
print(f"  heavier 10-50%:       {sum(1 for x in ratios if 1.1 < x < 1.5)}")
print(f"  heavier >=1.5x:       {sum(1 for x in ratios if x >= 1.5)}")
