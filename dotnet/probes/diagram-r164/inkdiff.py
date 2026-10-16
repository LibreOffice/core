#!/usr/bin/env python3
"""Whole-page pixel difference between the two renders, and the ink in a named region.

Settles whether the surplus text we draw is actually VISIBLE. Where a shape's overflow is
light text on the light page it is invisible, and the two pages can differ only in their
text layer; where it is dark it shows, and the defect is a rendering one as well.
"""
import pathlib, sys
import pymupdf
import census as C

DPI = 150


def gray(pdf, page=0):
    d = pymupdf.open(pdf)
    p = d[page].get_pixmap(dpi=DPI, colorspace=pymupdf.csGRAY)
    s, w, h, st = p.samples, p.width, p.height, p.stride
    d.close()
    return s, w, h, st


def main():
    here = pathlib.Path(__file__).parent
    rows = [l.split('\t') for l in (here / 'docs.tsv').read_text().split('\n') if l.strip()]
    print('stem\tdiffpx\tpx\tpct\tworstband_y0..y1(pt)\tbandpx')
    for stem, _ in rows:
        a = gray(C.only_pdf(str(here / 'ours' / stem)))
        b = gray(C.only_pdf(str(here / 'ref' / stem)))
        if (a[1], a[2]) != (b[1], b[2]):
            print('%s\tSIZE MISMATCH %sx%s vs %sx%s' % (stem[:44], a[1], a[2], b[1], b[2]))
            continue
        w, h, st = a[1], a[2], a[3]
        tot = 0
        rowdiff = []
        for y in range(h):
            ra = a[0][y*st:y*st+w]; rb = b[0][y*st:y*st+w]
            n = sum(1 for i in range(w) if abs(ra[i]-rb[i]) > 40)
            rowdiff.append(n); tot += n
        # worst 20-row band
        best, bi = -1, 0
        for i in range(0, max(1, h-20)):
            s = sum(rowdiff[i:i+20])
            if s > best: best, bi = s, i
        print('%s\t%d\t%d\t%.4f%%\t%.1f..%.1f\t%d'
              % (stem[:44], tot, w*h, 100.0*tot/(w*h),
                 bi*72/DPI, (bi+20)*72/DPI, best))


main()
