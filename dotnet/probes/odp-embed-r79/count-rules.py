#!/usr/bin/env python3
"""Count the thin wide fills on a page -- the marks an underline is drawn as.

Read out of the PDF's own drawing operators rather than from a raster, because a
mark the compositor discarded and a mark never drawn look identical in an image.
"""
import sys, pymupdf

def rules(pdf, page, max_h=2.5, min_w=20.0):
    d = pymupdf.open(pdf)
    n = 0
    for path in d[page].get_drawings():
        r = path["rect"]
        # Both fill and stroke: 26.2.4.2 strokes its underlines and this tree fills
        # them, which is the same rule drawn two ways and must not read as a difference.
        if r.height <= max_h and r.width >= min_w:
            n += 1
    return n

if __name__ == "__main__":
    for arg in sys.argv[1:]:
        pdf, _, page = arg.rpartition(":")
        print("%-70s %d" % (pdf.split("/")[-1] + " p" + page, rules(pdf, int(page) - 1)))
