#!/usr/bin/env python3
"""Which of two renderings of the same document differ in *pixels* rather than in bytes.

A clip emitted round a drawing block changes the content stream on every page that has one,
so a hash comparison reports as a mover every document that carries a drawing at all. What
matters is whether anything lands differently on the page, so this rasterises both sides and
reports the pages whose pixels differ, with the fraction of the page that does.

Usage: pixel-diff.py <dir-a> <dir-b> <identity...>   (or - to read identities on stdin)
"""
import pathlib
import sys

import pymupdf

DPI = 50


def pages(path):
    doc = pymupdf.open(path)
    for page in doc:
        pm = page.get_pixmap(dpi=DPI, colorspace=pymupdf.csGRAY)
        yield pm.width, pm.height, pm.samples


def compare(a, b):
    moved = 0
    total = 0
    worst = 0.0
    for (wa, ha, sa), (wb, hb, sb) in zip(pages(a), pages(b), strict=False):
        total += 1
        if (wa, ha) != (wb, hb):
            moved += 1
            worst = 1.0
            continue
        if sa == sb:
            continue
        differing = sum(1 for x, y in zip(sa, sb, strict=False) if x != y)
        if differing:
            moved += 1
            worst = max(worst, differing / max(len(sa), 1))
    return moved, total, worst


def main():
    a = pathlib.Path(sys.argv[1])
    b = pathlib.Path(sys.argv[2])
    names = sys.argv[3:]
    if names == ["-"] or not names:
        names = [line.strip() for line in sys.stdin if line.strip()]

    print("identity\tpages-differing\tpages\tworst-page-fraction")
    for ident in names:
        pa, pb = a / f"{ident}.pdf", b / f"{ident}.pdf"
        if not pa.exists() or not pb.exists():
            print(f"{ident}\t-\t-\tmissing")
            continue
        moved, total, worst = compare(pa, pb)
        print(f"{ident}\t{moved}\t{total}\t{worst:.5f}", flush=True)


if __name__ == "__main__":
    main()
