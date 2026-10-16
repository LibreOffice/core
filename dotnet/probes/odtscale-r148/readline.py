#!/usr/bin/env python3
"""The drawn width of each arm's single right-aligned line, from the PDF's own `Td`.

The page's right text edge is fixed, so the line's own origin is `edge − width`, and the width
is the difference. Read out of the text-positioning operator rather than from glyph boxes: that
channel is quantised to whole thousandths of an em and `dotnet/CLAUDE.md` records four rounds
lost to measuring a sub-thousandth effect through it.
"""
import glob
import os
import sys

import pymupdf


def origin(path):
    page = pymupdf.open(path)[0]
    spans = [s for b in page.get_text('dict')['blocks'] if b.get('lines')
             for l in b['lines'] for s in l['spans']]
    if len(spans) != 1:
        raise SystemExit('%s: expected one span, found %d' % (path, len(spans)))
    span = spans[0]
    return span['bbox'][0], span['bbox'][2], span['text']


def main():
    base = None
    for path in sorted(glob.glob(os.path.join(sys.argv[1] if len(sys.argv) > 1 else 'out',
                                              '*.pdf'))):
        left, right, text = origin(path)
        width = right - left
        if os.path.basename(path) == 'none.pdf':
            base = width
        print('%-8s width %8.3f pt   right edge %8.3f   %r'
              % (os.path.basename(path)[:-4], width, right, text))
    if base:
        print()
        for path in sorted(glob.glob(os.path.join(sys.argv[1] if len(sys.argv) > 1 else 'out',
                                                  '*.pdf'))):
            left, right, _ = origin(path)
            print('%-8s ratio to the unscaled arm %.5f'
                  % (os.path.basename(path)[:-4], (right - left) / base))


if __name__ == '__main__':
    main()
