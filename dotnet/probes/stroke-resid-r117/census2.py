#!/usr/bin/env python3
"""Round 113's census, plus the two columns that say what it is actually measuring.

`width_ratio` is round 113's and round 115's statistic: the ratio of the two sides' *mean*
stroke width over up to eight sampled pages, guarded so that both sides emit at least ten
stroked items with counts within 25 %.

Two more columns, because that mean is dominated by items of width **zero**.  A `0 w` stroke
in a PDF is a hairline — the thinnest line the device can draw, which for LibreOffice's own
export is a tenth of a point — and it enters the mean as nothing at all.  So:

  hair_ours / hair_ref   how many of each side's items are zero-width
  ratio_ex_hair          the same ratio with every zero-width item dropped

A document where the two disagree is one where the census is reading a difference in *which
items are hairlines*, not a difference in stroke weight.
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


def mean(xs):
    return sum(xs) / len(xs) if xs else 0.0


OURS = Path(sys.argv[1]); REF = Path(sys.argv[2]); OUT = Path(sys.argv[3])
rows = []
for ourpdf in sorted(OURS.glob("*.pdf")):
    refpdf = REF / ourpdf.name
    if not refpdf.exists():
        continue
    try:
        a = sample(ourpdf); b = sample(refpdf)
    except Exception as e:
        print("SKIP", ourpdf.name, e)
        continue
    if len(a) < 10 or len(b) < 10:
        continue
    if abs(len(a) - len(b)) > 0.25 * max(len(a), len(b)):
        continue
    ma, mb = mean(a), mean(b)
    ha = sum(1 for w in a if w < 1e-6)
    hb = sum(1 for w in b if w < 1e-6)
    ea = [w for w in a if w >= 1e-6]
    eb = [w for w in b if w >= 1e-6]
    rows.append((ourpdf.name, len(a), len(b), ma, mb, ma / mb if mb else 0.0,
                 ha, hb, (mean(ea) / mean(eb)) if eb and ea else 0.0))

with OUT.open("w") as f:
    f.write("doc\tours_items\tref_items\tours_meanw\tref_meanw\twidth_ratio\t"
            "hair_ours\thair_ref\tratio_ex_hair\n")
    for r in rows:
        f.write("%s\t%d\t%d\t%.3f\t%.3f\t%.3f\t%d\t%d\t%.3f\n" % r)

ratios = sorted(r[5] for r in rows)
n = len(ratios)
print(f"comparable {n}  median {ratios[n//2]:.3f}" if n else "comparable 0")
print(f"  ours lighter by >10%: {sum(1 for x in ratios if x < 0.9)}")
print(f"  within 10%:           {sum(1 for x in ratios if 0.9 <= x <= 1.1)}")
print(f"  heavier 10-50%:       {sum(1 for x in ratios if 1.1 < x < 1.5)}")
print(f"  heavier >=1.5x:       {sum(1 for x in ratios if x >= 1.5)}")
print(f"  mean |ratio-1|:       {mean([abs(x - 1) for x in ratios]):.4f}")
