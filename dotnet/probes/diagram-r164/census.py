#!/usr/bin/env python3
"""Outline census for the eight diagram/chart templates.

A run LibreOffice refuses to draw as text is decomposed to filled polygons
(VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D accepts a text primitive
only while abs(fontScaling.getY() * fShearX) < 1).  Those glyphs leave the text layer
entirely, so the page reads as though *we* invented text.  Counting glyph-sized filled
paths on both sides separates that ceiling from a real defect of ours.

Both sides are counted so the decorative small fills a page legitimately has (bullets,
line caps, tiny rules) cancel: the estimator for outlined glyphs is ref_fills - our_fills.
"""
import glob, pathlib, sys
import pymupdf

LIM = 20.0  # glyph-sized, in PDF points; overdraw.py's threshold


def only_pdf(d):
    f = sorted(glob.glob(d + '/*.pdf'))
    if len(f) != 1:
        raise SystemExit('expected one PDF under %s, found %d' % (d, len(f)))
    return f[0]


def small_fills(page, lim=LIM):
    n = 0
    for d in page.get_drawings():
        if d['type'] not in ('f', 'fs'):
            continue
        r = d['rect']
        if 0 < r.width < lim and 0 < r.height < lim:
            n += 1
    return n


def rotated_lines(page):
    n = 0
    for b in page.get_text('dict')['blocks']:
        if b['type'] != 0:
            continue
        for ln in b['lines']:
            if abs(ln['dir'][0] - 1.0) > 0.01:
                n += 1
    return n


def stats(pdf):
    doc = pymupdf.open(pdf)
    try:
        chars = fills = rot = 0
        for p in doc:
            chars += sum(1 for c in p.get_text() if c.isalnum())
            fills += small_fills(p)
            rot += rotated_lines(p)
        return chars, fills, rot, doc.page_count
    finally:
        doc.close()


def main():
    here = pathlib.Path(__file__).parent
    rows = [l.split('\t') for l in (here / 'docs.tsv').read_text().split('\n') if l.strip()]
    print('stem\tours\tref\tdelta\tref_fill\tour_fill\tfill_delta\tresidual\tour_rot\tref_rot\tpages')
    for stem, _ in rows:
        o = stats(only_pdf(str(here / 'ours' / stem)))
        r = stats(only_pdf(str(here / 'ref' / stem)))
        fd = r[1] - o[1]
        print('%s\t%d\t%d\t%+d\t%d\t%d\t%d\t%+d\t%d\t%d\t%d/%d'
              % (stem[:46], o[0], r[0], o[0] - r[0], r[1], o[1], fd,
                 o[0] - r[0] - fd, o[2], r[2], o[3], r[3]))


if __name__ == '__main__':
    main()
