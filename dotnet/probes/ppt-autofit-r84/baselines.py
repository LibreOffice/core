#!/usr/bin/env python3
"""Every drawn span of a page: its baseline, x origin, colour and text.

Read out of the PDF's own text with PyMuPDF rather than out of a raster, because
"is it absent" and "where is it" are both questions a raster cannot answer.
"""
import sys

try:                       # the `fitz` alias prints a deprecation line on stdout
    import pymupdf as fitz
except ImportError:
    import fitz

for path in sys.argv[1:]:
    doc = fitz.open(path)
    print(f'== {path}')
    for pno, page in enumerate(doc):
        d = page.get_text('dict')
        for block in d['blocks']:
            if block['type'] != 0: continue
            for line in block['lines']:
                for span in line['spans']:
                    print(f'  p{pno+1} y={span["origin"][1]:8.3f} x={span["origin"][0]:8.3f} '
                          f'sz={span["size"]:.2f} c={span["color"]:06x} {span["font"]}  {span["text"]!r}')
    doc.close()
