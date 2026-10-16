#!/usr/bin/env python3
"""Where 26.2.4.2 puts the one text line of each of the witness's two shape bodies.

Reports the span's bbox top/bottom and its baseline `origin`, and the face the reference
actually embedded, so a font substitution cannot pass unnoticed.
"""
import glob
import os
import sys

import pymupdf

WANT = ('Title:', 'Date:')


def rows(path):
    doc = pymupdf.open(path)
    out = []
    for pno, page in enumerate(doc):
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', ()):
                for span in line['spans']:
                    text = span['text'].strip()
                    if any(text.startswith(w) for w in WANT):
                        out.append((pno, text[:6], span['bbox'][1], span['bbox'][3],
                                    line['bbox'][1], line['bbox'][3], span['size'],
                                    span['font'], span['origin'][1]))
    return out


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'out'
    print('%-12s %-7s %8s %8s %8s %8s %-22s' %
          ('arm', 'text', 'top', 'bottom', 'baseline', 'size', 'font'))
    for path in sorted(glob.glob(os.path.join(out, '*.pdf'))):
        arm = os.path.basename(path)[:-4]
        found = rows(path)
        if not found:
            print('%-12s %-7s %8s' % (arm, '-', 'NO TEXT DRAWN'))
            continue
        for pno, text, top, bot, ltop, lbot, size, font, base in found:
            print('%-12s %-7s %8.3f %8.3f %8.3f %8.2f %-22s' %
                  (arm, text, top, bot, base, size, font))


if __name__ == '__main__':
    main()
