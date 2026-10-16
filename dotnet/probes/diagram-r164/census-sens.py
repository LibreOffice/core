#!/usr/bin/env python3
"""Threshold sensitivity for the outline census: if the reference were outlining glyphs,
some LIM would show a large ref-side excess of filled paths. Also total path counts."""
import pathlib, glob, pymupdf
import census as C

here = pathlib.Path(__file__).parent
rows = [l.split('\t') for l in (here / 'docs.tsv').read_text().split('\n') if l.strip()]
print('stem\t' + '\t'.join('L%d(ref/our)' % l for l in (8, 14, 20, 30, 50)) + '\ttotpaths(ref/our)')
for stem, _ in rows:
    out = []
    for lim in (8, 14, 20, 30, 50):
        vals = []
        for side in ('ref', 'ours'):
            d = pymupdf.open(C.only_pdf(str(here / side / stem)))
            vals.append(sum(C.small_fills(p, lim) for p in d)); d.close()
        out.append('%d/%d' % tuple(vals))
    tot = []
    for side in ('ref', 'ours'):
        d = pymupdf.open(C.only_pdf(str(here / side / stem)))
        tot.append(sum(len(p.get_drawings()) for p in d)); d.close()
    print('%s\t%s\t%d/%d' % (stem[:40], '\t'.join(out), tot[0], tot[1]))
