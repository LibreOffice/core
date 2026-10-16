#!/usr/bin/env python3
"""Measure a chart's plot rectangle from the long axis-parallel strokes on a page.

Gridlines and the axis lines are the only long, perfectly horizontal/vertical strokes on a
chart page.  Collect them, cluster by length, and report the extreme x and y of the family
that spans the plot.  Deliberately does NOT look at fills: the reference paints the wall as
one rectangle and we may not, so a fill census cannot compare the two stacks.
"""
import sys, re, collections
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
from pg import page_stream, npages

NUM = rb'-?\d*\.?\d+'
TOK = re.compile(rb'(' + NUM + rb'(?:\s+' + NUM + rb')*\s+[a-zA-Z*\'"]+|[a-zA-Z*\'"]+)')


def segments(stream):
    """Every straight stroked segment, in device space, with the CTM applied."""
    stream = re.sub(rb'BT.*?ET', b'', stream, flags=re.S)
    ctm = [1.0, 0.0, 0.0, 1.0, 0.0, 0.0]
    stack = []
    cur = []
    out = []
    start = None

    def app(m, x, y):
        return (m[0] * x + m[2] * y + m[4], m[1] * x + m[3] * y + m[5])

    def mul(a, b):
        return [a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
                a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
                a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5]]

    for t in TOK.finditer(stream):
        parts = t.group(1).split()
        op = parts[-1].decode('latin1')
        try:
            nums = [float(x) for x in parts[:-1]]
        except ValueError:
            continue
        if op == 'q':
            stack.append(list(ctm))
        elif op == 'Q':
            if stack: ctm = stack.pop()
        elif op == 'cm' and len(nums) >= 6:
            ctm = mul(nums[-6:], ctm)
        elif op == 'm' and len(nums) >= 2:
            cur = [app(ctm, nums[-2], nums[-1])]
        elif op == 'l' and len(nums) >= 2 and cur:
            cur.append(app(ctm, nums[-2], nums[-1]))
        elif op == 're' and len(nums) >= 4:
            x, y, w, h = nums[-4:]
            pts = [app(ctm, x, y), app(ctm, x + w, y),
                   app(ctm, x + w, y + h), app(ctm, x, y + h)]
            cur = pts + [pts[0]]
        elif op == 'c' and len(nums) >= 6 and cur:
            cur.append(app(ctm, nums[-2], nums[-1]))
        elif op in ('S', 's', 'B', 'b', 'B*', 'b*'):
            for a, b in zip(cur, cur[1:]):
                out.append((a, b))
            if op in ('s', 'b', 'b*') and len(cur) > 2:
                out.append((cur[-1], cur[0]))
            cur = []
        elif op in ('f', 'F', 'f*', 'n'):
            cur = []
    return out


def rect(segs, tol=0.05, minlen=20.0):
    hs = [(a, b) for a, b in segs if abs(a[1] - b[1]) < tol and abs(a[0] - b[0]) >= minlen]
    vs = [(a, b) for a, b in segs if abs(a[0] - b[0]) < tol and abs(a[1] - b[1]) >= minlen]
    return hs, vs


def report(label, path, page):
    s = page_stream(path, page)
    segs = segments(s)
    hs, vs = rect(segs)
    # the modal length family — a chart's grid all shares one length
    def modal(fam, ax):
        if not fam: return None
        lens = collections.Counter(round(abs(a[ax] - b[ax]), 1) for a, b in fam)
        L, n = lens.most_common(1)[0]
        keep = [(a, b) for a, b in fam if abs(round(abs(a[ax] - b[ax]), 1) - L) < 0.2]
        return L, n, keep
    print(f"--- {label} page {page+1}: {len(segs)} segs, {len(hs)} long-h, {len(vs)} long-v")
    mh = modal(hs, 0)
    if mh:
        L, n, keep = mh
        xs = [c[0] for a, b in keep for c in (a, b)]
        ys = sorted({round(a[1], 2) for a, b in keep})
        print(f"    h family len {L} x{n}  x {min(xs):.2f}..{max(xs):.2f}  "
              f"y {min(ys):.2f}..{max(ys):.2f}  ({len(ys)} distinct y)")
        print(f"      ys: {ys}")
    mv = modal(vs, 1)
    if mv:
        L, n, keep = mv
        ys = [c[1] for a, b in keep for c in (a, b)]
        xs = sorted({round(a[0], 2) for a, b in keep})
        print(f"    v family len {L} x{n}  y {min(ys):.2f}..{max(ys):.2f}  "
              f"x {min(xs):.2f}..{max(xs):.2f}  ({len(xs)} distinct x)")
        print(f"      xs: {xs}")


if __name__ == '__main__':
    page = int(sys.argv[3]) - 1
    report('ours', sys.argv[1], page)
    report('ref', sys.argv[2], page)
