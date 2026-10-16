#!/usr/bin/env python3
"""Mean absolute raster difference between two renderings of one page.

The measurement a gate column cannot make. A fill, an outline and a shape drawn at the
wrong size add no glyphs and no pages, so the only instrument that sees them is the
picture — and comparing the two *sides* rather than looking at either is what keeps it
from being a reading.

    inkgrid.py <a.pdf> <b.pdf> <1-based page> [dpi]

Prints the page mean and the worst cell of a 24 x 18 grid, in units of 0..255 per
channel summed over three channels (so 765 is black against white).
"""
import sys, pymupdf


def compare(a_path, b_path, page, dpi=72, gx=24, gy=18):
    a = pymupdf.open(a_path)[page].get_pixmap(dpi=dpi)
    b = pymupdf.open(b_path)[page].get_pixmap(dpi=dpi)
    if (a.width, a.height) != (b.width, b.height):
        raise SystemExit("page sizes differ: %dx%d against %dx%d"
                         % (a.width, a.height, b.width, b.height))

    sa, sb, w, h = a.samples, b.samples, a.width, a.height
    worst = 0
    total = 0
    count = 0
    for j in range(gy):
        for i in range(gx):
            cell = 0
            n = 0
            for y in range(j * h // gy, (j + 1) * h // gy, 3):
                base = y * w * 3
                for x in range(i * w // gx, (i + 1) * w // gx, 3):
                    k = base + x * 3
                    cell += (abs(sa[k] - sb[k]) + abs(sa[k + 1] - sb[k + 1])
                             + abs(sa[k + 2] - sb[k + 2]))
                    n += 3
            if n:
                worst = max(worst, int(cell / n))
                total += cell
                count += n
    return total / count if count else 0.0, worst


if __name__ == "__main__":
    mean, worst = compare(sys.argv[1], sys.argv[2], int(sys.argv[3]) - 1,
                          int(sys.argv[4]) if len(sys.argv) > 4 else 72)
    print("mean %.2f worst-cell %d" % (mean, worst))
