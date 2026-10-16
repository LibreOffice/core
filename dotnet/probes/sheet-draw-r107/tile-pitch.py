#!/usr/bin/env python3
"""How wide one pattern tile is drawn, on each side, measured off the two PDFs.

    tile-pitch.py <ours.pdf> <reference.pdf>

A pattern fill states no tile size, so the tile is the bitmap's own eight pixels at the device's
resolution — 96 dpi headless, which is 6 pt. That is the one number in O40's fix that is derived
rather than read out of the file, so it is measured on both sides here.

**Ours** tiles the 8x8 image as a grid of image placements, so the pitch is the step between two
consecutive `cm` origins on one row: read straight off `pdf-ops.py dump`.

**The reference** does not tile: a transparent bitmap fill is rasterised into one image with a
soft mask (`/Im212` on page 1 of `apron-area.xls`, 629x209 placed 151.910 pt wide). So its pitch
is the stripe period of that image, scaled by the placement — and one tile of the two-stripe
`00110011` blip is *two* periods.
"""
import re
import subprocess
import sys
import zlib

OPS = "/home/user/wt-sheetdraw/.claude/skills/render-comparison/scripts/pdf-ops.py"
PLACEMENT = re.compile(
    r"image\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(\S+)")


def our_pitch(pdf, page=1):
    """The x step between consecutive placements of one tiled image on one scanline."""
    dump = subprocess.run(["python3", OPS, "dump", pdf, "--page", str(page)],
                          capture_output=True, text=True, check=False).stdout
    rows = {}
    for line in dump.splitlines():
        found = PLACEMENT.match(line)
        if not found:
            continue
        x0, y0, x1, _ = (float(v) for v in found.groups()[:4])
        if (x1 - x0) > 12:                       # a placed picture, not a tile
            continue
        rows.setdefault((found.group(5), round(y0, 2)), []).append(x0)

    for (name, y), xs in sorted(rows.items()):
        xs.sort()
        if len(xs) < 3:
            continue
        print(f"ours\t{name}\ty={y}\t{len(xs)} tiles\t"
              f"pitch {(xs[-1] - xs[0]) / (len(xs) - 1):.4f} pt")


def reference_pitch(pdf, width=629, height=209, placed=151.910):
    """The stripe period of the reference's rasterised fill, in points."""
    data = open(pdf, "rb").read()
    found = re.search(
        rb"/Subtype/Image/Width %d/Height %d.*?stream\r?\n" % (width, height), data, re.S)
    if not found:
        print("reference\tno such image")
        return

    start = found.end()
    pixels = zlib.decompress(data[start:data.index(b"endstream", start)])
    for y in (50, 100, 150):
        row = [pixels[(((y * width) + x) * 3)] for x in range(width)]
        threshold = (min(row) + max(row)) / 2
        runs, previous, at = [], None, 0
        for x, value in enumerate(row + [255]):
            dark = value < threshold
            if dark != previous:
                if previous:
                    runs.append((at + x - 1) / 2)
                previous, at = dark, x
        period = (runs[-1] - runs[0]) / (len(runs) - 1)
        print(f"reference\tscanline {y}\t{len(runs)} stripes\t"
              f"period {period * placed / width:.4f} pt\t"
              f"tile {2 * period * placed / width:.4f} pt")


if __name__ == "__main__":
    our_pitch(sys.argv[1])
    reference_pitch(sys.argv[2])
