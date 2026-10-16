#!/usr/bin/env python3
"""Classify a remaining .odp gate row page by page, against a rendered pair.

For every page whose alphanumeric count differs, report the three signatures that tell the
three known ceilings apart from a defect of ours:

  images     - rasters actually placed on the page (`get_image_info`, not `get_images`,
               which answers the whole document's list and reads as identical on every page). The reference drawing one we do not is
               the raster ceiling (TODO.raster-ceiling.md).
  fills      - glyph-sized one-colour filled paths on the reference page. The reference having
               many where we draw rotated text is the *outlining* ceiling: LibreOffice refuses
               to draw a sheared text run and decomposes it to polygons
               (VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D,
               drawinglayer/source/processor2d/vclprocessor2d.cxx:126-141).
  rot        - text lines of ours whose direction is not (1, 0).

Asserts both sides produced a PDF before comparing anything.
"""
import glob, sys
import pymupdf


def pages(path):
    found = sorted(glob.glob(path + '/*.pdf'))
    if not found:
        raise SystemExit('no PDF under ' + path)
    return pymupdf.open(found[0])


def small_fills(page):
    return [d for d in page.get_drawings()
            if d['type'] == 'f' and d['rect'].width < 14 and d['rect'].height < 14]


def rotated_lines(page):
    n = 0
    for block in page.get_text('dict')['blocks']:
        if block['type'] != 0:
            continue
        for line in block['lines']:
            if abs(line['dir'][0] - 1.0) > 0.01:
                n += 1
    return n


def main():
    ref, ours = pages(sys.argv[1]), pages(sys.argv[2])
    print('pages ours %d ref %d' % (ours.page_count, ref.page_count))
    for i in range(min(ref.page_count, ours.page_count)):
        a = sum(1 for c in ref[i].get_text() if c.isalnum())
        b = sum(1 for c in ours[i].get_text() if c.isalnum())
        if a == b:
            continue
        print('  page %2d  ref %5d  ours %5d  %+5d   ref images %2d ours %2d   '
              'ref small fills %4d ours %4d   our rotated lines %3d'
              % (i + 1, a, b, b - a,
                 len(ref[i].get_image_info()), len(ours[i].get_image_info()),
                 len(small_fills(ref[i])), len(small_fills(ours[i])),
                 rotated_lines(ours[i])))


main()
