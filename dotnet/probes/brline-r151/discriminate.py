#!/usr/bin/env python3
"""Round 149's discriminating measurement, run with the tree quiescent.

Round 149 could measure nothing on our side -- the parent's build was live throughout -- and named
the one render that would settle where the defect is. This is it. The reference column is that
round's banked `measured3.txt` / `measured4.txt`; ours is the same quantity read from our own PDFs
with that round's own `read3.py`, so one instrument reads both halves.

  b*        the BREAK RUN's size varied   -> does the break line take the run's size?
  m*        the PARAGRAPH MARK's varied   -> does it take the mark's, as the seat claimed?
  n0..n6    0..6 breaks, default size     -> what one break costs
  pr259-n*  0..4 breaks at 108 % spacing  -> is the proportional surplus applied twice?
"""
import pathlib
import re


def table(path):
    out = {}
    for line in pathlib.Path(path).read_text().splitlines()[1:]:
        parts = line.split()
        if len(parts) >= 3:
            try:
                out[parts[0]] = float(parts[2])
            except ValueError:
                pass
    return out


FAMILIES = [
    (r'b\d+', 'the BREAK RUN size, in half-points'),
    (r'm\d+', 'the PARAGRAPH MARK size, in half-points'),
    (r'n\d', '0..6 breaks at the default size'),
    (r'pr259-n\d', '0..4 breaks in a 108 per cent paragraph'),
]


def main():
    here = pathlib.Path(__file__).resolve().parent
    ref = {}
    for name in ('measured3.txt', 'measured4.txt'):
        ref.update(table(here.parent / 'brline-r149' / name))
    ours = table(here / 'ours3.txt')

    for pattern, label in FAMILIES:
        arms = sorted((a for a in ref if re.fullmatch(pattern, a) and a in ours),
                      key=lambda a: (len(a), a))
        print('== %s  (%s)' % (pattern, label))
        for arm in arms:
            print('   %-10s reference %8.3f   ours %8.3f   %s'
                  % (arm, ref[arm], ours[arm],
                     'agree' if abs(ref[arm] - ours[arm]) <= 0.05 else 'DIFFER'))
        print()


if __name__ == '__main__':
    main()
