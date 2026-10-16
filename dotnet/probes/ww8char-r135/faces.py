#!/usr/bin/env python3
"""Glyphs drawn per font face, for one document rendered two ways and for the reference.

    faces.py <base-dir> <head-dir> <ref-dir> <identity>...

A weight change draws the same characters in a different face, so no gate column can see it and
`|ink|%` only says a region moved. The face histogram says WHICH glyphs changed weight and lets the
two legs be compared against the reference's own histogram term by term.
"""
import collections
import pathlib
import sys

import pymupdf


def faces(pdf):
    counts = collections.Counter()
    with pymupdf.open(pdf) as doc:
        for page in doc:
            for block in page.get_text('rawdict')['blocks']:
                if block['type'] != 0:
                    continue
                for line in block['lines']:
                    for span in line['spans']:
                        counts[span['font']] += len(span['chars'])
    return counts


base, head, ref = (pathlib.Path(p) for p in sys.argv[1:4])
print('document\tface\tbase\thead\treference')
for name in sys.argv[4:]:
    b = sorted((base / name).glob('*.pdf'))
    h = sorted((head / name).glob('*.pdf'))
    r = ref / (name + '.pdf')
    if not b or not h or not r.exists():
        print('%s\tMISSING' % name)
        continue
    cb, ch, cr = faces(b[0]), faces(h[0]), faces(r)
    for face in sorted(set(cb) | set(ch) | set(cr)):
        print('%s\t%s\t%d\t%d\t%d' % (name, face, cb[face], ch[face], cr[face]))
