#!/usr/bin/env python3
"""Per-page mean absolute grey difference between two of our renderings and a reference.

A frame that moves adds no glyphs and no pages, so no gate column can see it and the score has
to be taken off the pixels. Both banks are compared against the same reference render, so the
number is a *distance* that can go either way rather than a diff that can only grow.

Usage:  python3 score-ink.py <ref.pdf> <before.pdf> <after.pdf> [dpi]
"""
import subprocess, sys, tempfile
from pathlib import Path

import numpy as np
from PIL import Image


def pagecount(pdf):
    out = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith("Pages:"):
            return int(line.split()[1])
    return 0


def page(pdf, n, dpi):
    with tempfile.TemporaryDirectory() as t:
        stem = Path(t) / "p"
        subprocess.run(["pdftoppm", "-r", str(dpi), "-gray", "-png", "-f", str(n), "-l", str(n),
                        "-singlefile", str(pdf), str(stem)], capture_output=True)
        f = Path(str(stem) + ".png")
        return np.asarray(Image.open(f), dtype=np.int16) if f.exists() else None


def diff(a, b):
    h, w = min(a.shape[0], b.shape[0]), min(a.shape[1], b.shape[1])
    return float(np.abs(a[:h, :w] - b[:h, :w]).mean())


def main():
    ref, before, after = (Path(p) for p in sys.argv[1:4])
    dpi = int(sys.argv[4]) if len(sys.argv) > 4 else 100
    n = min(pagecount(ref), pagecount(before), pagecount(after))
    print(f"{ref.stem}: pages ref {pagecount(ref)} before {pagecount(before)} after {pagecount(after)}")
    b_all, a_all = [], []
    for i in range(1, n + 1):
        r, b, a = page(ref, i, dpi), page(before, i, dpi), page(after, i, dpi)
        if r is None or b is None or a is None:
            continue
        db, da = diff(b, r), diff(a, r)
        b_all.append(db)
        a_all.append(da)
        print(f"  page {i:3d}  ink before {db:6.3f}  after {da:6.3f}  delta {da - db:+7.3f}")
    if b_all:
        print(f"  MEAN      ink before {sum(b_all)/len(b_all):6.3f}  "
              f"after {sum(a_all)/len(a_all):6.3f}  "
              f"delta {(sum(a_all)-sum(b_all))/len(b_all):+7.3f}  over {len(b_all)} pages")


main()
