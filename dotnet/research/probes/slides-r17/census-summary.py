#!/usr/bin/env python3
"""Summarise a `size-census.py` run: how many page-level size disagreements, and of what size.

The point of the split is that two very different things land in the same column. A
disagreement of a twentieth of a per cent is the draw layer's 1/100 mm grid; one of several
per cent is the shrink-to-fit search landing on a different candidate, which is a real and
much larger defect. Counting them together says nothing about either.

The second filter is the raster ceiling. On a page the reference rasterises, its dominant
size carries one or two show operators against our dozens, and the "disagreement" is that
it drew no text at all. Requiring the reference to draw at least half as many operators as
we do removes those without needing the page list.

    census-summary.py <census-output.txt>
"""
import re
import sys

ROW = re.compile(r'page\s+(\d+)\s+ours\s+([\d.]+) x(\d+)\s+ref\s+([\d.]+) x(\d+).*'
                 r'\(([+-][\d.]+)%\)')
HEAD = re.compile(r'^(\S.*): (\d+) of (\d+) pages differ')


def main(path):
    total = small = big = comparable = 0
    per_doc = {}
    current = None
    for line in open(path, encoding='utf-8'):
        head = HEAD.match(line)
        if head:
            current = head.group(1)
            continue
        row = ROW.search(line)
        if not row:
            continue
        ours_n, ref_n = int(row.group(3)), int(row.group(5))
        pct = abs(float(row.group(6)))
        total += 1
        if pct <= 1.0:
            small += 1
            continue
        big += 1
        if ref_n >= 0.5 * ours_n:
            comparable += 1
            per_doc[current] = per_doc.get(current, 0) + 1

    print(f'pages differing on the dominant /Tf size   {total}')
    print(f'  by <= 1%  — the 1/100 mm grid            {small}')
    print(f'  by  > 1%                                  {big}')
    print(f'     of those, both sides drawing text      {comparable}'
          f'  over {len(per_doc)} documents')
    print('\nworst documents:')
    for doc, n in sorted(per_doc.items(), key=lambda kv: -kv[1])[:20]:
        print(f'  {n:4d}  {doc}')


if __name__ == '__main__':
    main(sys.argv[1])
