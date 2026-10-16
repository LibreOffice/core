#!/usr/bin/env python3
"""Page count and alphanumeric count for every mover, both legs and the reference.

    gate-columns.py <movers.tsv> <base-dir> <after-dir> <ref-dir> <out.tsv>

The two columns the corpus gate scores. A rule is ink: if either of them moves, this round did
something it did not intend to.
"""
import pathlib
import sys

import pymupdf

movers, base, after, ref, out = sys.argv[1:6]
base, after, ref = (pathlib.Path(p) for p in (base, after, ref))


def columns(path):
    if path is None or not path.exists():
        return None
    with pymupdf.open(path) as d:
        text = ''.join(p.get_text() for p in d)
        pages = d.page_count
    return pages, sum(1 for c in text if c.isalnum())


def only(d):
    pdfs = sorted(d.glob('*.pdf')) if d.exists() else []
    return pdfs[0] if pdfs else None


changed = 0
rows = 0
with open(out, 'w') as fh:
    fh.write('identity\tbase_pages\tafter_pages\tref_pages\tbase_alnum\tafter_alnum\tref_alnum\n')
    for line in pathlib.Path(movers).read_text().splitlines()[1:]:
        ident, verdict = line.split('\t')
        if verdict != 'moved':
            continue
        b, a = columns(only(base / ident)), columns(only(after / ident))
        r = columns(ref / (ident + '.pdf'))
        if b is None or a is None:
            continue
        rows += 1
        if b != a:
            changed += 1
        fh.write('%s\t%d\t%d\t%s\t%d\t%d\t%s\n'
                 % (ident, b[0], a[0], r[0] if r else '', b[1], a[1], r[1] if r else ''))
print('%d movers scored, %d changed a page or alphanumeric count' % (rows, changed))
