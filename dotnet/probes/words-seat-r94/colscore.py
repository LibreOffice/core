#!/usr/bin/env python3
"""How far a rendering's *left edges* sit from the reference's, per page, weighted by ink.

`odt-startx-r88/startx.py` answers a different question and cannot answer this one: it merges
every span of a baseline and keeps the leftmost, so on a two-column page it only ever samples the
first column, and a section drawn from the wrong left edge in the second column moves nothing in
it.  It also matches lines by `difflib`, which pairs identical repeated labels in draw order.

This matches nothing textual.  Per page, every text span contributes its own left edge; edges
within `tol` are one cluster; each of our clusters is paired with the reference cluster nearest to
it, and the page's score is the span-count-weighted mean |dx| of those pairs, plus the count of
our spans that have no reference cluster within `far` (an edge the reference does not use at all).

    colscore.py <ours-dir> <ref-dir> <list-of-pdf-names>
"""
import os, sys, collections, pymupdf

TOL = 1.0
FAR = 3.0

def page_clusters(path):
    pages = collections.defaultdict(list)
    with pymupdf.open(path) as doc:
        for pno, page in enumerate(doc):
            for b in page.get_text('dict')['blocks']:
                for l in b.get('lines', []):
                    for s in l['spans']:
                        if s['text'].strip():
                            pages[pno].append(s['origin'][0])
    out = {}
    for pno, xs in pages.items():
        xs.sort()
        cl = []
        for x in xs:
            if cl and x - cl[-1][-1] <= TOL:
                cl[-1].append(x)
            else:
                cl.append([x])
        out[pno] = [(sum(c) / len(c), len(c)) for c in cl]
    return out

def score(ours, ref):
    a, b = page_clusters(ours), page_clusters(ref)
    tot = n = orphan = 0.0
    for pno in sorted(set(a) & set(b)):
        refx = [x for x, _ in b[pno]]
        if not refx:
            continue
        for x, w in a[pno]:
            d = min(abs(x - r) for r in refx)
            tot += d * w
            n += w
            if d > FAR:
                orphan += w
    return (tot / n if n else 0.0), int(orphan), int(n)

def main():
    ours_a, ours_b, refdir, listing = sys.argv[1:5]
    names = [l.strip() for l in open(listing) if l.strip().endswith('.pdf')]
    print(f"{'mean|dx| before':>15} {'after':>9} {'off-edge before':>16} {'after':>7} "
          f"{'spans':>7}   document")
    sa = sb = sn = 0.0
    for nm in names:
        r = os.path.join(refdir, nm)
        if not os.path.exists(r):
            print(f"{'':>15} {'':>9} {'':>16} {'':>7} {'':>7}   {nm}  (no reference)")
            continue
        d0, o0, n0 = score(os.path.join(ours_a, nm), r)
        d1, o1, _ = score(os.path.join(ours_b, nm), r)
        flag = '  BETTER' if d1 < d0 - 1e-9 else ('  WORSE' if d1 > d0 + 1e-9 else '')
        print(f"{d0:15.3f} {d1:9.3f} {o0:16d} {o1:7d} {n0:7d}   {nm}{flag}")
        sa += d0 * n0; sb += d1 * n0; sn += n0
    if sn:
        print(f"{sa / sn:15.3f} {sb / sn:9.3f} {'':>16} {'':>7} {int(sn):7d}   TOTAL")

main()
