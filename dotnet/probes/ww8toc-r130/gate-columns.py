#!/usr/bin/env python3
"""The two gate columns a rendering change could move, over one sweep directory.

    gate-columns.py <base-dir> <after-dir> <ref-dir> <out.tsv>

Pages and ALPHANUMERIC CHARACTERS — column 9 of a gate row, the column the verdict is decided in
within max(2 %, 15) — for both of our legs and for the banked reference, so that "no verdict can
move" is a measurement rather than an argument from what the change touches.
"""
import pathlib
import sys

import pymupdf


def columns(path):
    with pymupdf.open(path) as d:
        text = ''.join(d[i].get_text() for i in range(d.page_count))
        return d.page_count, sum(1 for c in text if c.isalnum())


base, after, refdir, out = (pathlib.Path(p) for p in sys.argv[1:5])
moved = 0
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\tpages_base\tpages_after\tpages_ref\tglyphs_base\tglyphs_after\tglyphs_ref\n')
    for d in sorted(p for p in base.iterdir() if p.is_dir()):
        a = sorted(d.glob('*.pdf'))
        b = sorted((after / d.name).glob('*.pdf'))
        ref = refdir / (d.name + '.pdf')
        if not a or not b or not ref.exists():
            continue
        pa, ga = columns(a[0])
        pb, gb = columns(b[0])
        pr, gr = columns(ref)
        if (pa, ga) != (pb, gb):
            moved += 1
        fh.write('%s\t%d\t%d\t%d\t%d\t%d\t%d\n' % (d.name, pa, pb, pr, ga, gb, gr))

print('documents whose page or alphanumeric count moved between the two legs: %d' % moved)
