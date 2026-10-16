#!/usr/bin/env python3
"""What moved between two legs of the same sweep, and by how much.

    confine.py <base-dir> <head-dir> <out.tsv>

Both directories are `sweep-doc.py` output — one subdirectory per document, one PDF in each,
`SOURCE_DATE_EPOCH` pinned so an unchanged rendering is byte-identical. Three columns per
document, because a change that moves no page and no glyph is a change the gate cannot see and
the byte comparison can:

  identical   the two PDFs are the same bytes
  pages       page count on each leg
  alnum       count of alphanumeric characters extracted from each leg

The point of the run is the *first* column over the whole track: confinement is how few
documents move, not how well the ones that do.
"""
import hashlib
import pathlib
import sys

import pymupdf


def only_pdf(d):
    pdfs = sorted(d.glob('*.pdf'))
    return pdfs[0] if pdfs else None


def measure(pdf):
    with pymupdf.open(pdf) as doc:
        pages = doc.page_count
        alnum = sum(sum(c.isalnum() for c in p.get_text()) for p in doc)
    return pages, alnum


base, head, out = (pathlib.Path(p) for p in sys.argv[1:4])
rows = []
for d in sorted(p for p in base.iterdir() if p.is_dir()):
    a, b = only_pdf(d), only_pdf(head / d.name)
    if a is None or b is None:
        rows.append((d.name, 'missing', '', '', '', ''))
        continue
    same = hashlib.sha256(a.read_bytes()).digest() == hashlib.sha256(b.read_bytes()).digest()
    if same:
        rows.append((d.name, 'yes', '', '', '', ''))
        continue
    pa, na = measure(a)
    pb, nb = measure(b)
    rows.append((d.name, 'no', pa, pb, na, nb))

with out.open('w') as fh:
    fh.write('document\tidentical\tpages_base\tpages_head\talnum_base\talnum_head\n')
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')

moved = [r for r in rows if r[1] == 'no']
print('%d of %d move' % (len(moved), len(rows)))
print('page counts that move: %d' % sum(1 for r in moved if r[2] != r[3]))
print('alnum counts that move: %d' % sum(1 for r in moved if r[4] != r[5]))
for r in moved:
    print('  %s pages %s->%s alnum %s->%s' % (r[0], r[2], r[3], r[4], r[5]))
