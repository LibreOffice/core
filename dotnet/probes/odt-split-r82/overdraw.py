#!/usr/bin/env python3
"""Text against *outlined* glyphs, for the templates the gate says we over-draw.

A run LibreOffice decomposes to filled polygons is absent from the text layer, so a document whose
reference outlines part of its shape text reads as though this tree drew more than the reference —
when what differs is the representation.  `VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D`
(`drawinglayer/source/processor2d/vclprocessor2d.cxx`:126-141) accepts a text primitive only while
`abs(fontScaling.getY() * fShearX) < 1` and decomposes everything else.

Counting both columns on both sides is one measurement and it separates the two.
"""
import pathlib
import sys

import pymupdf


def counts(pdf):
    """(alphanumeric characters in the text layer, glyph-sized filled paths)."""
    doc = pymupdf.open(pdf)
    text = outlined = 0
    try:
        for page in doc:
            text += sum(1 for c in page.get_text() if c.isalnum())
            outlined += sum(
                1 for d in page.get_drawings()
                if d['rect'].width < 20 and d['rect'].height < 20)
    finally:
        doc.close()
    return text, outlined


if __name__ == '__main__':
    ours = pathlib.Path(sys.argv[1])
    bank = pathlib.Path(sys.argv[2] if len(sys.argv) > 2 else '/home/user/gate-odf-r78/ref')
    print('document\toursText\toursOutlined\trefText\trefOutlined')
    for pdf in sorted(ours.glob('*.pdf')):
        reference = bank / f'{pdf.stem}__odt.pdf'
        if not reference.exists():
            continue
        o = counts(pdf)
        r = counts(reference)
        print(f'{pdf.stem}\t{o[0]}\t{o[1]}\t{r[0]}\t{r[1]}')
