#!/usr/bin/env python3
"""Split a `size-census.py` run three ways, because its ">1%" column is not one class.

`census-summary.py` (round seventeen) separates the 1/100 mm quantisation — since fixed —
from everything larger, and filters the raster ceiling out by requiring the reference to
draw at least half as many show operators as we do. What is left was read as "a size
disagreement of more than one per cent on a page where both sides draw comparable text",
and treated as one body of work.

It is not one body of work. Its top-ranked document by page count, `2014BSA_Sunday_Killion`,
has pages where the reference draws 208 operators at 11.400 pt against our 45 at 14.980 —
and 11.400 pt is *not a size the draw layer can hold*. A slide's character height lives in
an `SvxFontHeightItem` in the model's map unit, a hundredth of a millimetre, so every size
the reference resolves through the shape model is `n * 72/2540`. A size off that grid did
not come through the shape model: it was played out of a metafile, or rasterised. Those
pages are the ceiling wearing a different hat, and no amount of work on inheritance or on
the autofit search will move them.

So the split here is on the reference's own dominant size:

    on the 1/100 mm grid      a size the shape model resolved — ours to get right
    off it                    metafile or rasterised text — the ceiling

    census-split.py <census-output.txt>
"""
import collections
import re
import sys

ROW = re.compile(r'page\s+(\d+)\s+ours\s+([\d.]+) x(\d+)\s+ref\s+([\d.]+) x(\d+).*'
                 r'\(([+-][\d.]+)%\)')
HEAD = re.compile(r'^(\S.*): (\d+) of (\d+) pages differ')

MM100_PER_PT = 2540.0 / 72.0
TOL_PT = 0.004        # half the 0.01 pt the PDF writes, so it cannot mask a real miss


def on_grid(size):
    mm = size * MM100_PER_PT
    return abs(mm - round(mm)) <= TOL_PT * MM100_PER_PT


def main(path):
    total = small = big = 0
    model = collections.Counter()
    metafile = collections.Counter()
    ceiling = 0
    pairs = collections.Counter()
    current = None

    for line in open(path, encoding='utf-8'):
        head = HEAD.match(line)
        if head:
            current = head.group(1)
            continue
        row = ROW.search(line)
        if not row:
            continue
        ours, ours_n = float(row.group(2)), int(row.group(3))
        ref, ref_n = float(row.group(4)), int(row.group(5))
        pct = abs(float(row.group(6)))

        total += 1
        if pct <= 1.0:
            small += 1
            continue
        big += 1
        if ref_n < 0.5 * ours_n:
            ceiling += 1
            continue
        if on_grid(ref):
            model[current] += 1
            pairs[(round(ours, 2), round(ref, 2))] += 1
        else:
            metafile[current] += 1

    print(f'pages differing on the dominant /Tf size    {total}')
    print(f'  by <= 1%  — the 1/100 mm grid             {small}')
    print(f'  by  > 1%                                  {big}')
    print(f'     the reference draws far less there     {ceiling}   raster ceiling')
    print(f'     its size is off the 1/100 mm grid      {sum(metafile.values())}'
          f'   metafile text, over {len(metafile)} documents')
    print(f'     its size is a size the model resolved  {sum(model.values())}'
          f'   over {len(model)} documents')

    print('\nworst documents, model-resolved sizes only:')
    for doc, n in sorted(model.items(), key=lambda kv: -kv[1])[:20]:
        print(f'  {n:4d}  {doc}')

    print('\nworst documents, metafile text (not winnable by resolution work):')
    for doc, n in sorted(metafile.items(), key=lambda kv: -kv[1])[:10]:
        print(f'  {n:4d}  {doc}')

    print('\ncommonest (ours, ref) pairs among the model-resolved:')
    for (a, b), n in pairs.most_common(20):
        print(f'  {n:4d}  ours {a:7.2f}  ref {b:7.2f}   ratio {a / b:.4f}')


if __name__ == '__main__':
    main(sys.argv[1])
