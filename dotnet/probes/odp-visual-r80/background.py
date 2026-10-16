#!/usr/bin/env python3
"""Sample a page's background colour down both sides, and census its drawing operators.

Two questions an image cannot separate: whether a mark is absent or drawn in a colour
that vanishes, and whether a gradient's stops, its interpolation or its extent is wrong.
The first is answered by counting the page's own operators; the second by reading the
colour at fractional heights at an x that avoids text.
"""
import sys, pymupdf

def profile(pdf, page, xs=(0.02, 0.98), rows=11):
    d = pymupdf.open(pdf)
    p = d[page]
    pix = p.get_pixmap(dpi=72)
    out = []
    for f in range(rows):
        y = min(pix.height - 1, int(round(f * (pix.height - 1) / (rows - 1))))
        row = []
        for x in xs:
            px = min(pix.width - 1, int(round(x * (pix.width - 1))))
            row.append(pix.pixel(px, y))
        out.append((f / (rows - 1), row))
    return out

def census(pdf, page):
    d = pymupdf.open(pdf)
    p = d[page]
    kinds = {}
    for path in p.get_drawings():
        k = (path["type"], len(path["items"]))
        kinds[k] = kinds.get(k, 0) + 1
    return kinds, len(p.get_images(full=True)), len(p.get_drawings())

if __name__ == "__main__":
    for arg in sys.argv[1:]:
        pdf, _, page = arg.rpartition(":")
        page = int(page) - 1
        kinds, images, drawings = census(pdf, page)
        print("==", pdf.split("/")[-1], "page", page + 1,
              "drawings", drawings, "images", images)
        for f, row in profile(pdf, page):
            print("   %4.2f  %s" % (f, "  ".join("#%02x%02x%02x" % c for c in row)))
