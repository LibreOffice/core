#!/usr/bin/env python3
"""Read the drawn lines out of the PDF's own content stream, WITHOUT honouring clip paths.

PyMuPDF's structured-text extraction applies the page's clip paths, so a line that 26.2.4.2
emits under a clip rectangle comes back as missing characters or as no text at all.  That read
says "the reference dropped the text" when the reference drew it and clipped it, and it cost
this round one wrong section.  Here the operators are parsed directly: every `Td`/`Tm` origin,
the `Tf` size, the number of glyph codes in the `TJ` array, and every `re W* n` clip rectangle.
"""
import glob
import os
import re
import sys

import pymupdf

TEXT = re.compile(r'([-\d.]+)\s+([-\d.]+)\s+Td\s*/F(\d+)\s+([\d.]+)\s+Tf\s*\[(.*?)\]\s*TJ', re.S)
CLIP = re.compile(r'q\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+re\s*\nW\*?\s+n')
CODES = re.compile(r'<([0-9A-Fa-f]+)>')


def read(path):
    page = pymupdf.open(path)[0]
    raw = page.read_contents().decode('latin1')
    h = page.rect.height
    lines = []
    for x, y, fid, size, arr in TEXT.findall(raw):
        n = sum(len(c) // 2 for c in CODES.findall(arr))
        lines.append((round(h - float(y), 3), round(float(x), 2), float(size), n))
    clips = [(round(float(a), 2), round(h - float(b) - float(d), 2), round(float(c), 2),
              round(float(d), 2))
             for a, b, c, d in CLIP.findall(raw)
             if not (float(c) > 500 and float(d) > 500)]
    return sorted(lines), clips


def main():
    for d in sys.argv[1:] or ['out5']:
        for path in sorted(glob.glob(os.path.join(d, '*.pdf'))):
            lines, clips = read(path)
            print('%-14s %-64s clips=%s' % (
                os.path.basename(path)[:-4],
                ' '.join('y%.2f/x%.0f/%gpt/%dg' % l for l in lines),
                clips))


if __name__ == '__main__':
    main()
