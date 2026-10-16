#!/usr/bin/env python3
"""First page where the two renderings' text diverges, with the geometry of that page."""
import sys, pathlib, pymupdf

def pages(p):
    d = pymupdf.open(p); out=[]
    for pg in d:
        w = pg.get_text("words")
        t = "".join(x[4] for x in w)
        ys = sorted({round(x[1],1) for x in w})
        lines=[]; prev=None
        for y in ys:
            if prev is None or y-prev>1.5: lines.append(y); prev=y
        out.append((t, lines, pg.rect.height, w))
    return out

for name in sys.argv[1:]:
    a = pages(f"/home/user/gate-odf-r76/ours/{name}__rtf.pdf")
    b = pages(f"/home/user/gate-odf-r76/ref/{name}__rtf.pdf")
    k = None
    for i in range(min(len(a),len(b))):
        if a[i][0] != b[i][0]: k = i; break
    print(f"### {name}   ours {len(a)}p  ref {len(b)}p   first divergent page: {k+1 if k is not None else 'none'}  pageH={a[0][2]}")
    if k is None: continue
    for tag, r in (("ours", a[k]), ("ref ", b[k])):
        ls = r[1]
        print(f"  {tag}: {len(ls)} lines  y {ls[0]:.2f} .. {ls[-1]:.2f}  gaps(last5)={[round(ls[i+1]-ls[i],2) for i in range(max(0,len(ls)-6),len(ls)-1)]}")
    # what is on ours' page k that ref has and vice versa
    ta, tb = a[k][0], b[k][0]
    n = 0
    while n < min(len(ta),len(tb)) and ta[n]==tb[n]: n += 1
    print(f"  common prefix {n} chars; ours tail {len(ta)-n!r} ref tail {len(tb)-n!r}")
    print(f"  ours after: {ta[n:n+90]!r}")
    print(f"  ref  after: {tb[n:n+90]!r}")
