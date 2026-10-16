#!/usr/bin/env python3
"""Report each page's text band and its baseline pitch, so two renderings can be compared
for how much body a page holds rather than for what is on it."""
import sys, pymupdf
doc = pymupdf.open(sys.argv[1])
lo = int(sys.argv[2]) - 1 if len(sys.argv) > 2 else 0
hi = int(sys.argv[3]) if len(sys.argv) > 3 else min(len(doc), lo + 10)
for pno in range(lo, hi):
    p = doc[pno]
    ys = []
    for b in p.get_text('dict')['blocks']:
        for l in b.get('lines', []):
            for s in l['spans']:
                ys.append(round(s['origin'][1], 2))
    ys = sorted(set(ys))
    if not ys:
        print(f"p{pno+1}: empty"); continue
    gaps = [round(ys[i+1]-ys[i], 2) for i in range(len(ys)-1)]
    small = [g for g in gaps if g < 40]
    print(f"p{pno+1}: {len(ys)} baselines  y {ys[0]:.2f}..{ys[-1]:.2f}  "
          f"median gap {sorted(small)[len(small)//2] if small else 0:.2f}  "
          f"page h {p.rect.height:.1f}")
