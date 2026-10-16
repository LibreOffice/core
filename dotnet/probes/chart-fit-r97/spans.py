#!/usr/bin/env python3
"""Every drawn text span of two PDFs, paired in drawn order, with the displacement between them.

O23 is stated as a displacement of the pie's data labels, so a displacement is what scores it —
`pdf-image-diff.py`'s ink answers "how much differs", not "by how far", and on a chart whose
whole content shifts the two are not the same question.

Usage: spans.py OURS.pdf REFERENCE.pdf [page]
"""
import sys

import pymupdf


def spans(path, page):
    out = []
    for block in pymupdf.open(path)[page].get_text('dict')['blocks']:
        for line in block.get('lines', []):
            for s in line['spans']:
                if s['text'].strip():
                    out.append((s['bbox'][0], s['bbox'][1], s['size'], s['text'].strip()))
    return sorted(out, key=lambda r: (round(r[1], 1), round(r[0], 1)))


def main():
    page = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    a, b = spans(sys.argv[1], page), spans(sys.argv[2], page)
    print(f'ours {len(a)} spans, reference {len(b)}')
    print(f'{"dx":>8} {"dy":>8}  {"ours x":>8} {"ref x":>8}  {"ours y":>8} {"ref y":>8}  text')

    # Paired on the *text*, not on the index: one unmatched span shifts an index pairing by one
    # and turns an agreeing page into a table of hundred-point displacements.
    left = list(b)
    worst = 0.0
    for x in a:
        hit = next((y for y in left if y[3] == x[3]), None)
        if hit is None:
            print(f'{"":>8} {"":>8}  ours only{"":>32}  {x[3][:60]}')
            continue
        left.remove(hit)
        dx, dy = x[0] - hit[0], x[1] - hit[1]
        worst = max(worst, abs(dx), abs(dy))
        print(f'{dx:8.2f} {dy:8.2f}  {x[0]:8.2f} {hit[0]:8.2f}  {x[1]:8.2f} {hit[1]:8.2f}  {x[3][:60]}')
    for y in left:
        print(f'{"":>8} {"":>8}  reference only{"":>27}  {y[3][:60]}')
    print(f'worst |dx| or |dy| over paired spans: {worst:.2f} pt')


main()
