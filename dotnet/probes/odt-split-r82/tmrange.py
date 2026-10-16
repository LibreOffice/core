#!/usr/bin/env python3
"""The vertical range a PDF's text operators actually cover, page by page.

PyMuPDF's `get_text` — and `pdftotext` — report only what falls on the sheet, so text drawn below
the page's bottom edge is invisible to both.  A frame that should have been split across pages
draws its tail there, which is why a glyph count reads *missing* for a defect that is *unsplit*.
This reads the content stream's own `Tm`/`Td` operands instead, so nothing is clipped away.
"""
import pathlib
import re
import sys

import pymupdf

TM = re.compile(rb'([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+Tm')


def span(pdf):
    doc = pymupdf.open(pdf)
    worst = None
    for number, page in enumerate(doc):
        stream = b''.join(doc.xref_stream(x) for x in page.get_contents())
        ys = [float(m.group(6)) for m in TM.finditer(stream)]
        if not ys:
            continue
        # PDF user space has y up from the bottom of the sheet, so a negative y is below it.
        low, high = min(ys), max(ys)
        if worst is None or low < worst[1]:
            worst = (number, low, high, page.rect.height)
    doc.close()
    return worst


if __name__ == '__main__':
    root = pathlib.Path(sys.argv[1])
    print('document\tworstpage\tlowestTm\thighestTm\tpageheight')
    for pdf in sorted(root.glob('*.pdf')):
        got = span(pdf)
        if got is None or got[1] >= 0:
            continue
        print(f'{pdf.stem}\t{got[0]}\t{got[1]:.1f}\t{got[2]:.1f}\t{got[3]:.1f}')
