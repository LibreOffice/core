#!/usr/bin/env python3
"""How many corpus slides pages draw a bullet, and how far off its vertical placement is.

The fitted bullet's 1.9 pt has been carried as an open item for seven rounds without anyone
measuring its *reach*.  This measures it off the two renderings rather than off the model: a
bullet is a one- or two-glyph run whose pen sits to the left of a longer run on nearly the same
line, so pair each short run with the first longer run within 60 pt to its right and 4 pt of its
own y, and report the marker-minus-text baseline offset on each side.

A page counts as carrying the defect when both stacks pair the same number of markers and the
mean |ours - ref| offset exceeds a quarter of a point.
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
        if not 1 <= n <= 2:
            continue
        best = None
        for x2, y2, a2, em2, t2 in rs:
            if x2 <= x + 1 or x2 - x > 60 or abs(y2 - y) > 4:
                continue
            n2 = len(t2.strip('<>')) // 2 if t2.startswith('<') else len(t2)
            if n2 < 4:
                continue
            if best is None or x2 < best[0]:
                best = (x2, y2)
        if best:
            out.append((round(x, 1), y - best[1]))
    return out


if __name__ == '__main__':
    ours, ref = sys.argv[1], sys.argv[2]
    pages = docs = 0
    off = []
    bad = collections.Counter()
    for name in sorted(os.listdir(ours)):
        if not name.endswith('.pdf'):
            continue
        o, r = os.path.join(ours, name), os.path.join(ref, name)
        if not os.path.exists(r):
            continue
        try:
            no, nr = npages(o), npages(r)
        except Exception:
            continue
        if no != nr:
            continue
        hit = False
        for p in range(no):
            try:
                a, b = markers(o, p), markers(r, p)
            except Exception:
                continue
            if not a or len(a) != len(b):
                continue
            pages += 1
            d = [abs(x[1] - y[1]) for x, y in zip(sorted(a), sorted(b))]
            m = sum(d) / len(d)
            off.append(m)
            if m > 0.25:
                bad[name[:-4]] += 1
                hit = True
        if hit:
            docs += 1
    off.sort()
    print(f"{pages} pages pair markers on both sides in {len(os.listdir(ours))} documents")
    print(f"{sum(bad.values())} of them are over 0.25 pt out, in {docs} documents")
    if off:
        print(f"median |d| {off[len(off)//2]:.3f} pt   mean {sum(off)/len(off):.3f}   max {off[-1]:.3f}")
    for k, v in bad.most_common(20):
        print(f"   {v:4d} pages   {k}")
