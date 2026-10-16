#!/usr/bin/env python3
"""Print one page's drawn lines with their boxes, so a wrap can be read rather than eyeballed.

    ./lines.py <pdf> [page, 1-based] [substring]

PyMuPDF's line box includes the trailing blank's advance, which is the whole point here: a line
the reference ends with a space reports a right edge one space beyond its last glyph, and
comparing that against the placeholder's own right edge is what says whether the blank was
charged against the break.
"""
import sys
import pymupdf

doc = pymupdf.open(sys.argv[1])
page = doc[int(sys.argv[2]) - 1 if len(sys.argv) > 2 else 0]
want = sys.argv[3] if len(sys.argv) > 3 else ''

for block in page.get_text('dict')['blocks']:
    for line in block.get('lines', []):
        text = ''.join(span['text'] for span in line['spans'])
        if not text.strip() or want not in text:
            continue
        x0, y0, x1, y1 = line['bbox']
        print(f'({x0:7.2f},{y0:7.2f})-({x1:7.2f},{y1:7.2f}) w={x1 - x0:7.2f}  {text!r}')
