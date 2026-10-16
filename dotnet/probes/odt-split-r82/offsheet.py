#!/usr/bin/env python3
"""How much of a document's text is drawn outside its own sheet.

A frame taller than the room left on its page draws its tail below the page's bottom edge, where
`pdftotext` cannot see it — so a glyph count reads *missing* where the defect is *unsplit*.  This
reads the rendering's own text boxes instead and reports, per document, how many characters land
outside the page rectangle and how far past it the lowest one is.
"""
import pathlib
import sys

import pymupdf


def offsheet(pdf):
    doc = pymupdf.open(pdf)
    out = below = 0
    lowest = 0.0
    for page in doc:
        h, w = page.rect.height, page.rect.width
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                text = ''.join(s['text'] for s in line['spans'])
                n = sum(1 for c in text if c.isalnum())
                x0, y0, x1, y1 = line['bbox']
                if y1 > h or y0 < 0 or x1 > w or x0 < 0:
                    out += n
                    if y1 > h:
                        below += n
                        lowest = max(lowest, y1 - h)
    doc.close()
    return out, below, lowest


if __name__ == '__main__':
    root = pathlib.Path(sys.argv[1])
    names = sys.argv[2:]
    print('document\toutside\tbelow\tlowest')
    for pdf in sorted(root.glob('*.pdf')):
        if names and not any(n in pdf.name for n in names):
            continue
        out, below, lowest = offsheet(pdf)
        if out:
            print(f'{pdf.stem}\t{out}\t{below}\t{lowest:.1f}')
