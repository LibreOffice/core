#!/usr/bin/env python3
"""Shifted or unshifted, read off the rendering: the category labels' own spacing.

On a shifted axis n categories occupy n slots and the first label's centre sits half a slot
inside the plot's left edge; unshifted, n categories are n points and the first and last labels'
centres sit ON the edges.  Both are decidable from the label pen positions and the plot rectangle
alone, with no font metrics: the ratio (last centre - first centre) / plot width is
(n-1)/n shifted and 1 unshifted.  The label's own width cancels because every label here is the
same width.
"""
import sys, collections
sys.path.insert(0, '/c/sandbox/workdir/scratch-r59-slides/plot')
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
sys.path.insert(0, '/c/sandbox/workdir/scratch-r60-slides')
from gridrect import segments
from textpos import texts
from pg import page_stream


def plot(stream):
    segs = segments(stream)
    hs = [(a, b) for a, b in segs if abs(a[1]-b[1]) < 0.05 and abs(a[0]-b[0]) >= 40]
    if not hs:
        return None
    c = collections.Counter(round(abs(a[0]-b[0]), 1) for a, b in hs)
    L, n = c.most_common(1)[0]
    keep = [(a, b) for a, b in hs if abs(round(abs(a[0]-b[0]), 1) - L) < 0.2]
    xs = [p[0] for a, b in keep for p in (a, b)]
    ys = [p[1] for a, b in keep for p in (a, b)]
    return min(xs), max(xs), min(ys), max(ys)


def row(label, path, page=0):
    st = page_stream(path, page)
    p = plot(st)
    if p is None:
        print(f"{label:20s} no plot"); return
    x0, x1, y0, y1 = p
    # the category label row: the text run band just below the plot, most populous y
    ts = [t for t in texts(st) if y0 - 25 < t[1] < y0 and x0 - 30 < t[0] < x1 + 30]
    if len(ts) < 3:
        print(f"{label:20s} plot {x0:7.2f}..{x1:7.2f}  only {len(ts)} labels"); return
    ts.sort()
    band = collections.Counter(round(t[1], 1) for t in ts).most_common(1)[0][0]
    ts = [t for t in ts if abs(t[1] - band) < 0.5]
    pens = [t[0] for t in ts]
    n = len(pens)
    span = pens[-1] - pens[0]
    width = x1 - x0
    print(f"{label:20s} plot {x0:7.2f}..{x1:7.2f} w{width:7.2f}  {n} labels  "
          f"span/width {span/width:6.4f}  expect shifted {(n-1)/n:6.4f} unshifted 1.0000  "
          f"first pen {pens[0]:7.2f}")


if __name__ == '__main__':
    for a in sys.argv[1:]:
        row(a.split('/')[-1][:-4], a)
