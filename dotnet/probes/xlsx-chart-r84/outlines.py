#!/usr/bin/env python3
"""Count each side's glyph-sized filled paths, per page.

A chart label the reference turns off a right angle inside an anisotropically
squeezed chart is decomposed to filled polygons rather than drawn as text
(`VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D`,
`drawinglayer/source/processor2d/vclprocessor2d.cxx`:126-141), so it is absent
from every text channel.  A row where the reference outlines and we draw text is a
measured ceiling, not a defect: no gate column can score our better output.
"""
import sys
import pymupdf


def glyph_fills(pdf):
    doc = pymupdf.open(pdf)
    out = []
    for pg in doc:
        n = 0
        for d in pg.get_drawings():
            if d['type'] not in ('f', 'fs'):
                continue
            r = d['rect']
            if 0.5 < r.width < 30 and 1 < r.height < 30:
                n += 1
        out.append(n)
    return out


if __name__ == '__main__':
    a, b = glyph_fills(sys.argv[1]), glyph_fills(sys.argv[2])
    print('page\tours\tref')
    for i in range(max(len(a), len(b))):
        print(f'{i+1}\t{a[i] if i < len(a) else "-"}\t{b[i] if i < len(b) else "-"}')
    print(f'TOTAL\t{sum(a)}\t{sum(b)}')
