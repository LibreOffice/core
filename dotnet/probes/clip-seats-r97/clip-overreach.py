#!/usr/bin/env python3
"""Does the block clip cut ink 26.2.4.2 keeps?

Three renderings of the same page — the reference, ours before the clip landed and ours
after — answer the question the ink ranking cannot. A pixel counts as *over-clipped* when
the reference inks it, our unclipped rendering inked it, and our clipped one does not:
that is ink the clip removed and the reference keeps, and it is the only evidence that
our clip rectangle is wrong rather than merely exposing some other difference.

The complement is *under-clipped*: our clipped rendering still inks it and neither the
reference nor a correct clip would.  Reported beside it so a page can be told apart from
one where the clip simply did not reach far enough.

    clip-overreach.py REFDIR BEFOREDIR AFTERDIR NAME...
"""
import sys, os
import numpy as np
import pymupdf

DPI = 120
T = 200  # 8-bit grey below this counts as ink

def gray(path, pno):
    d = pymupdf.open(path); p = d[pno]
    pix = p.get_pixmap(dpi=DPI, colorspace=pymupdf.csGRAY)
    a = np.frombuffer(pix.samples, dtype=np.uint8).reshape(pix.height, pix.width)
    d.close()
    return a

def main(refdir, beforedir, afterdir, names):
    print("document\tpage\tover_px\tover_pct\tover_x0\tover_x1\tunder_px\tunder_pct")
    for name in names:
        paths = [os.path.join(d, name + ".pdf") for d in (refdir, beforedir, afterdir)]
        if not all(os.path.exists(p) for p in paths): 
            print(f"# missing {name}", file=sys.stderr); continue
        docs = [pymupdf.open(p) for p in paths]
        n = min(d.page_count for d in docs)
        same = len({d.page_count for d in docs}) == 1
        for d in docs: d.close()
        if not same:
            print(f"# page counts differ for {name}", file=sys.stderr); continue
        for i in range(n):
            R, B, A = (gray(p, i) for p in paths)
            if R.shape != B.shape or R.shape != A.shape: continue
            ri, bi, ai = R < T, B < T, A < T
            over = ri & bi & ~ai
            under = ai & ~ri
            tot = R.size
            if over.sum() == 0 and under.sum() == 0: continue
            cols = np.nonzero(over.any(axis=0))[0]
            x0 = cols[0] * 72.0 / DPI if len(cols) else -1
            x1 = cols[-1] * 72.0 / DPI if len(cols) else -1
            print(f"{name}\t{i+1}\t{over.sum()}\t{100.0*over.sum()/tot:.3f}\t"
                  f"{x0:.1f}\t{x1:.1f}\t{under.sum()}\t{100.0*under.sum()/tot:.3f}")

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:])
