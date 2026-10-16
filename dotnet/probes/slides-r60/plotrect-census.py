#!/usr/bin/env python3
"""Every chart page in a sweep, with our plot rectangle against the reference's.

The plot rectangle is read off the *gridlines*, not off a fill: both stacks draw a family of
long, exactly axis-parallel strokes across the plot and nothing else on a slide does.  A page
qualifies only when BOTH sides show at least four members of one length family in the same
direction, so a page where one side draws no grid at all is skipped rather than reported as a
displacement of the whole frame.
"""
import os, sys, collections
sys.path.insert(0, '/c/sandbox/workdir/scratch-r59-slides/plot')
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
from gridrect import segments
from pg import page_stream, npages


def box(path, page):
    try:
        segs = segments(page_stream(path, page))
    except Exception:
        return None
    hs = [(a, b) for a, b in segs if abs(a[1]-b[1]) < 0.05 and abs(a[0]-b[0]) >= 40]
    vs = [(a, b) for a, b in segs if abs(a[0]-b[0]) < 0.05 and abs(a[1]-b[1]) >= 40]
    if len(hs) + len(vs) < 4:
        return None

    def fam(f, ax):
        if not f: return None
        c = collections.Counter(round(abs(a[ax]-b[ax]), 1) for a, b in f)
        L, n = c.most_common(1)[0]
        if n < 4: return None
        keep = [(a, b) for a, b in f if abs(round(abs(a[ax]-b[ax]), 1) - L) < 0.2]
        return keep

    kh, kv = fam(hs, 0), fam(vs, 1)
    pts = [c for k in (kh or []) + (kv or []) for c in k]
    if len(pts) < 8: return None
    return (min(p[0] for p in pts), min(p[1] for p in pts),
            max(p[0] for p in pts), max(p[1] for p in pts),
            len(kh or []), len(kv or []))


if __name__ == '__main__':
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    rows = []
    for name in sorted(os.listdir(ours_dir)):
        if not name.endswith('.pdf'): continue
        o = os.path.join(ours_dir, name)
        r = os.path.join(ref_dir, name)
        if not os.path.exists(r): continue
        try:
            no, nr = npages(o), npages(r)
        except Exception:
            continue
        if no != nr: continue
        for pg in range(no):
            bo, br = box(o, pg), box(r, pg)
            if bo is None or br is None: continue
            # both sides must show a comparable family
            rows.append((name[:-4], pg + 1,
                         bo[0]-br[0], bo[1]-br[1], bo[2]-br[2], bo[3]-br[3],
                         br[2]-br[0], br[3]-br[1], bo[4], bo[5], br[4], br[5]))
    print("doc\tpage\tdLeft\tdBottom\tdRight\tdTop\trefW\trefH\toH\toV\trH\trV")
    for row in rows:
        print("%s\t%d\t%+.2f\t%+.2f\t%+.2f\t%+.2f\t%.1f\t%.1f\t%d\t%d\t%d\t%d" % row)
    print(f"# {len(rows)} chart pages", file=sys.stderr)
