#!/usr/bin/env python3
"""Count pairs of pictures drawn on one baseline, one over the other.

    baseline-pairs.py <dir> <out.tsv> [--flat]

The defect's own signature, measured on the drawn page rather than on the markup: two images on
one page whose BOTTOM edges agree to a tenth of a point and whose horizontal extents overlap by
more than half the narrower one. Two as-character pictures that share a line rest on the same
baseline, so their bottoms coincide exactly; two pictures stacked down the page do not.

`--flat` reads one `<identity>.pdf` per document (a gate bank); without it, one directory per
document (`sweep-doc.py`'s layout).

The reference's own count over the same population is the base rate, and it is what says whether
a coincidence of bottom edges means anything at all.
"""
import pathlib
import sys

import pymupdf

BOTTOM = 0.1


def images(page):
    return [b['bbox'] for b in page.get_text('rawdict')['blocks'] if b['type'] == 1]


def pairs(page):
    boxes = images(page)
    found = 0
    for i in range(len(boxes)):
        for j in range(i + 1, len(boxes)):
            a, b = boxes[i], boxes[j]
            if abs(a[3] - b[3]) > BOTTOM:
                continue
            overlap = min(a[2], b[2]) - max(a[0], b[0])
            narrower = min(a[2] - a[0], b[2] - b[0])
            if narrower > 0 and overlap > narrower / 2:
                found += 1
    return found


root, out = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
flat = '--flat' in sys.argv[3:]

if flat:
    items = [(p.stem, p) for p in sorted(root.glob('*.pdf'))]
else:
    items = []
    for d in sorted(p for p in root.iterdir() if p.is_dir()):
        pdfs = sorted(d.glob('*.pdf'))
        if pdfs:
            items.append((d.name, pdfs[0]))

documents = total = 0
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\tpages\tpairs\n')
    for name, path in items:
        try:
            with pymupdf.open(path) as doc:
                count = sum(pairs(doc[i]) for i in range(doc.page_count))
                pages = doc.page_count
        except Exception as exc:                       # noqa: BLE001
            print('%s: %s' % (name, type(exc).__name__), file=sys.stderr)
            continue
        fh.write('%s\t%d\t%d\n' % (name, pages, count))
        if count:
            documents += 1
            total += count

print('%d of %d documents draw a coincident overlapping pair, %d pairs in all'
      % (documents, len(items), total))
