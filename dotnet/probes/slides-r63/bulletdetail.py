#!/usr/bin/env python3
"""Per-page detail behind bullet-census.py: the two sizes and both offsets.

The census reports a mean |d| per page and nothing about *why*.  This reports, for every
paired marker, the marker's em, the text run's em, and each side's marker-minus-text baseline
offset -- which is what separates "the rule is wrong" from "one of its inputs is".
"""
import os, sys, collections
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
sys.path.insert(0, '/c/sandbox/workdir/scratch-r62-slides')
from rotruns import runs
from pg import page_stream, npages

def markers(path, page):
    rs = [r for r in runs(page_stream(path, page)) if abs(r[2]) < 1.0]
    out = []
    for x, y, a, em, t in rs:
        n = len(t.strip('<>')) // 2 if t.startswith('<') else len(t)
        if not 1 <= n <= 2: continue
        best = None
        for x2, y2, a2, em2, t2 in rs:
            if x2 <= x + 1 or x2 - x > 60 or abs(y2 - y) > 4: continue
            n2 = len(t2.strip('<>')) // 2 if t2.startswith('<') else len(t2)
            if n2 < 4: continue
            if best is None or x2 < best[0]: best = (x2, y2, em2)
        if best: out.append((round(x, 1), y - best[1], em, best[2]))
    return out

if __name__ == '__main__':
    ours, ref = sys.argv[1], sys.argv[2]
    print("doc\tpage\tk\tourBulEm\tourTxtEm\trefBulEm\trefTxtEm\tourOff\trefOff\td")
    for name in sorted(os.listdir(ours)):
        if not name.endswith('.pdf'): continue
        o, r = os.path.join(ours, name), os.path.join(ref, name)
        if not os.path.exists(r): continue
        try:
            no, nr = npages(o), npages(r)
        except Exception: continue
        if no != nr: continue
        for p in range(no):
            try: a, b = markers(o, p), markers(r, p)
            except Exception: continue
            if not a or len(a) != len(b): continue
            a, b = sorted(a), sorted(b)
            d = sum(abs(x[1] - y[1]) for x, y in zip(a, b)) / len(a)
            if d <= 0.25: continue
            x, y = a[0], b[0]
            print("%s\t%d\t%d\t%.3f\t%.3f\t%.3f\t%.3f\t%+.3f\t%+.3f\t%+.3f" % (
                name[:-4], p + 1, len(a), x[2], x[3], y[2], y[3], x[1], y[1], x[1] - y[1]))
