#!/usr/bin/env python3
"""Test whether every `/Tf` size a rendering uses is a whole number of 1/100 mm.

The draw layer holds a character height as an `SvxFontHeightItem` in the model's map unit,
which for Impress is 1/100 mm, so a reference `/Tf` size should always be `n * 72/2540`
for integer `n`. If that holds on the reference and fails on ours, our em is carrying
precision the reference cannot represent — which is a systematic advance-width error on
every run, and the kind of thing that decides an autofit near-tie.

    mm100-grid.py <sweepdir> [--limit N]
"""
import argparse
import collections
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'slides-r15'))
from pdfops import dump  # noqa: E402

MM100_PER_PT = 2540.0 / 72.0
TOL = 0.004        # half of the 0.01 pt the PDF writes, so it cannot mask a real miss


def used(path, max_pages=200):
    out = collections.Counter()
    page = 1
    while page <= max_pages:
        try:
            ops = dump(path, page)
        except (IndexError, KeyError, ValueError, ZeroDivisionError):
            break
        for font, _base, sz, _tm, _td in ops:
            if font != 'TL' and sz:
                out[round(sz, 4)] += 1
        page += 1
    return out


def on_grid(size):
    mm = size * MM100_PER_PT
    return abs(mm - round(mm)) <= TOL * MM100_PER_PT


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('sweepdir')
    ap.add_argument('--limit', type=int, default=0)
    args = ap.parse_args()

    tally = {'ours': [0, 0], 'ref': [0, 0]}      # [on grid, total], weighted by operators
    offenders = collections.Counter()
    names = sorted(n for n in os.listdir(os.path.join(args.sweepdir, 'ours'))
                   if n.endswith('.pdf'))
    if args.limit:
        names = names[:args.limit]
    for name in names:
        for side in ('ours', 'ref'):
            p = os.path.join(args.sweepdir, side, name)
            if not os.path.exists(p):
                continue
            try:
                counts = used(p)
            except Exception:                     # noqa: BLE001 — a probe, not a library
                continue
            for size, n in counts.items():
                tally[side][1] += n
                if on_grid(size):
                    tally[side][0] += n
                elif side == 'ours':
                    offenders[size] += n

    for side, (ok, total) in tally.items():
        pct = 100.0 * ok / total if total else 0.0
        print(f'{side:5s}  {ok:8d} of {total:8d} show operators sit on the '
              f'1/100 mm grid  ({pct:.2f}%)')
    print('\nthe sizes ours writes that the reference could not hold, '
          'commonest first:')
    for size, n in offenders.most_common(15):
        print(f'  {size:9.3f} pt  = {size * MM100_PER_PT:9.3f} mm100   x{n}')


if __name__ == '__main__':
    main()
