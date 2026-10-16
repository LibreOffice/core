#!/usr/bin/env python3
"""Each arm's drawn width, in paragraph order, from the PDF's own span boxes.

Every arm draws the same word, so the widths are directly comparable and the ratio is exact. The
first five paragraphs are right aligned and hold one word each; the sixth is left aligned and holds
two, an unscaled one and a scaled one, which is the uniform-paragraph shortcut's arm.
"""
import sys

import pymupdf

LABELS = ['absent', 'hundred', 'ninetynine', 'sixty', 'onethirty', 'plain', 'span']


def widths(path):
    out = []
    for page in pymupdf.open(path):
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                for span in line['spans']:
                    if span['text'].strip():
                        out.append((span['bbox'][2] - span['bbox'][0], span['text']))
    return out


def main():
    rows = widths(sys.argv[1])
    if len(rows) != len(LABELS):
        raise SystemExit('%s: expected %d spans, found %d: %r'
                         % (sys.argv[1], len(LABELS), len(rows), [t for _, t in rows]))
    base = rows[0][0]
    for label, (width, text) in zip(LABELS, rows):
        print('%-12s width %8.3f   ratio %.5f   %r' % (label, width, width / base, text))
    # Against the ABSENT arm, not against `plain` beside it: the reference's `plain` span carries
    # the trailing blank before the span, so it measures 86.712 rather than 83.712 and a ratio taken
    # against it reads 0.57899 for a 60 % span. Comparing like with like is the point of every arm
    # drawing the same word.
    print('%-12s ratio to the absent arm %.5f' % ('span', rows[6][0] / base))


if __name__ == '__main__':
    main()
