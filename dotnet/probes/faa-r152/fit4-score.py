#!/usr/bin/env python3
"""Threshold per k from fixture 4, read off the WIDEST line rather than the line count.

`read-fit.py`'s line count is not a safe observable here: a superscript sits on its own baseline
and PyMuPDF sometimes merges the two lines of a wrapped cell into one. The widest drawn line is
unambiguous -- it jumps to the full single-line width exactly when the cell stops wrapping.
"""
import csv, sys
sys.path.insert(0, '.')
from hmtx import Face

f = Face('/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf')
KS = [1, 2, 3, 4, 6, 8, 12, 16]


def load(p):
    return {r['arm']: (int(r['lines']), float(r['widest_pt'])) for r in csv.DictReader(open(p), delimiter='\t')}


ref, our = load(sys.argv[1]), load(sys.argv[2])
print('k\trefW\toursW\tdiff')
t = {}
for k in KS:
    ws = sorted(int(a.split('_')[1]) for a in ref if a.startswith(f'P{k}_'))
    def thr(d):
        top = max(d[f'P{k}_{w}'][1] for w in ws)
        for w in ws:
            if abs(d[f'P{k}_{w}'][1] - top) < 1e-6:
                return w
        return None
    t[k] = (thr(ref), thr(our))
    print(f'{k}\t{t[k][0]}\t{t[k][1]}\t{t[k][1]-t[k][0]:+d}')
print()
per19 = f.twips('19', 1.0)
print('per-"19" advance from the slope, and the superscript size it implies:')
for lo, hi in ((1, 16), (2, 16), (4, 16), (8, 16)):
    for idx, name in ((0, 'ref'), (1, 'ours')):
        s = (t[hi][idx] - t[lo][idx]) / (hi - lo)
        print(f'  k {lo}->{hi}  {name:4}  {s:8.4f} tw   size {s/per19:.5f} pt   {s/per19/8*100:7.3f} % of 8 pt')
print()
print('predicted W(k) for candidate reference sizes (base "i " = 80 tw, margins 216):')
for size in (4.60, 4.6325, 4.64, 4.65):
    row = [216 + int(-(-(80.0 + k * f.twips('19', size)) // 1)) for k in KS]
    print(f'  {size:.4f}: {row}')
print('  observed ref :', [t[k][0] for k in KS])
print('  observed ours:', [t[k][1] for k in KS])
