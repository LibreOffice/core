#!/usr/bin/env python3
"""Filled and stroked paths on one PDF page, counted by the colour they were issued in.

`pdf-ops.py` pairs shapes between two renderings; `textcolour.py` reports the colour of text.
Neither answers *"how many filled rectangles does this page carry, in what colours"*, which is the
question a table shade and a cell border are. Colour is tracked through `rg`/`g`/`k` with `q`/`Q`
saving and restoring it, the same way `textcolour.py` does for the fill colour.

Two bugs in the first cut, both of which inflated the count and neither of which looked wrong:
whitespace was tokenised as an operator, so every run of numbers was discarded before the operator
that consumed it and the whole page came back black; and **the letters inside a hex string are
tokens too**, so every `<…F…>` glyph string was counted as the legacy `F` fill operator. A page
reported 95 fills where its stream holds 75. Strings are now removed before tokenising and the
count is checked against a plain `grep -c` of the two painting operators.

    fillcount.py <pdf> [page]
"""
import collections
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from textcolour import page_streams  # noqa: E402

TOKEN = re.compile(rb'[-+\d.]+|/[^\s/\[\]<>(){}]+|[A-Za-z*\']+')
HEX = re.compile(rb'<[^>]*>')
LITERAL = re.compile(rb'\((?:\\.|[^\\()])*\)', re.S)
FILLS = ('f', 'F', 'f*', 'b', 'b*', 'B', 'B*')
STROKES = ('S', 's', 'b', 'b*', 'B', 'B*')


def hexof(c):
    return '#%02X%02X%02X' % tuple(max(0, min(255, round(x * 255))) for x in c)


def counts(path, page):
    stream = page_streams(open(path, 'rb').read(), int(page))
    stream = LITERAL.sub(b'()', HEX.sub(b'<>', stream))
    fills = collections.Counter()
    strokes = collections.Counter()
    fill = stroke = (0.0, 0.0, 0.0)
    stack = []
    nums = []
    for raw in TOKEN.findall(stream):
        if re.fullmatch(rb'[-+\d.]+', raw):
            try:
                nums.append(float(raw))
            except ValueError:
                nums = []
            continue
        op = raw.decode('latin-1')
        if op == 'q':
            stack.append((fill, stroke))
        elif op == 'Q':
            if stack:
                fill, stroke = stack.pop()
        elif op == 'rg' and len(nums) >= 3:
            fill = tuple(nums[-3:])
        elif op == 'g' and nums:
            fill = (nums[-1],) * 3
        elif op == 'RG' and len(nums) >= 3:
            stroke = tuple(nums[-3:])
        elif op == 'G' and nums:
            stroke = (nums[-1],) * 3
        if op in FILLS:
            fills[hexof(fill)] += 1
        if op in STROKES:
            strokes[hexof(stroke)] += 1
        nums = []
    return fills, strokes


def main(path, page=1):
    fills, strokes = counts(path, page)
    print('%s page %s' % (path.rsplit('/', 1)[-1], page))
    print('  fills   %3d  %s' % (sum(fills.values()), dict(fills.most_common())))
    print('  strokes %3d  %s' % (sum(strokes.values()), dict(strokes.most_common())))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else 1)
