#!/usr/bin/env python3
"""The vertical range a PDF's text-showing operators actually cover, page by page.

PyMuPDF's ``get_text`` — and ``pdftotext`` — report only what falls on the sheet, so text drawn
below the page's bottom edge is invisible to both.  A frame that should have been split across
pages draws its tail there, which is why a glyph count can read *missing* for a defect that is
*unsplit*.  This reads the content stream's own text-positioning operands instead, ``Td``/``TD``
and ``Tm`` alike, so nothing is clipped away.
"""
import pathlib
import re
import sys

import pymupdf

TD = re.compile(rb'([-\d.]+)\s+([-\d.]+)\s+T[dD]\b')
TM = re.compile(rb'([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+Tm')


def ys(stream):
    out = [float(m.group(2)) for m in TD.finditer(stream)]
    out += [float(m.group(6)) for m in TM.finditer(stream)]
    return out


def span(pdf):
    """(page, lowest, highest, page height) for the page whose text reaches lowest."""
    doc = pymupdf.open(pdf)
    worst = None
    try:
        for number, page in enumerate(doc):
            stream = b''.join(doc.xref_stream(x) for x in page.get_contents())
            found = ys(stream)
            if not found:
                continue
            if worst is None or min(found) < worst[1]:
                worst = (number, min(found), max(found), page.rect.height)
    finally:
        doc.close()
    return worst


if __name__ == '__main__':
    root = pathlib.Path(sys.argv[1])
    print('document\tworstpage\tlowestTd\thighestTd\tpageheight')
    for pdf in sorted(root.glob('*.pdf')):
        got = span(pdf)
        if got is None or got[1] >= 0:
            continue
        print(f'{pdf.stem}\t{got[0]}\t{got[1]:.1f}\t{got[2]:.1f}\t{got[3]:.1f}')
