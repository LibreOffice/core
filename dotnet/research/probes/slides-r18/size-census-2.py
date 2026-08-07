#!/usr/bin/env python3
"""`size-census.py` with the two false-positive classes its ranking is blind to removed.

Round seventeen's census compares the *dominant* `/Tf` size per page, weighting each size
by how many show operators carry it. That is the right weighting for finding an autofit
disagreement in a wall of body text, and it has two blind spots that between them account
for a large share of what it reports.

**Operator granularity.** The dominant size is decided by a count of show operators, and
the two writers do not split a line into operators the same way. Measured on
`ws_prod-g-doc-Events-2007-september-M.017-(French)-France.ppt` page 3, where the census
reports *ours 20.010 x7 against ref 10.010 x8, +99.90%* — a doubling, and the largest
single ratio class in the whole run. The two distributions are:

    ours  [(31.02, 2), (28.01, 1), (20.01, 7), (15.99, 3), (10.01, 5)]
    ref   [           (28.01, 1), (20.01, 7), (15.99, 3), (10.01, 8)]

Every size agrees. The reference splits the slide-number footer into four portions at
x = 635.6, 639.1, 645.8 and 649.3 where we write one at 638.7, and those three extra
operators are enough to move its dominant size from 20 pt to 10 pt. Five decks of that
family carry ~40 such pages between them.

**Metafile text.** A size the reference resolved through the shape model is a whole number
of hundredths of a millimetre, since that is the unit an `SvxFontHeightItem` holds for a
draw object. A reference dominant size off that grid was played out of a metafile or
rasterised, and no work on inheritance or on the autofit search will move it.

A page is kept here only when the dominant size of one side is genuinely *absent* from the
other — fewer than half as many operators carry it there — which is what separates "the two
renderers chose different sizes" from "they chose the same sizes and cut them up
differently".

    size-census-2.py <sweepdir> [--filter SUBSTRING] [--verbose]
"""
import argparse
import collections
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'slides-r15'))
from pdfops import dump  # noqa: E402

MM100_PER_PT = 2540.0 / 72.0
TOL_PT = 0.004
SHARE = 0.5              # "absent from the other side" means fewer than half as many


def on_grid(size):
    mm = size * MM100_PER_PT
    return abs(mm - round(mm)) <= TOL_PT * MM100_PER_PT


def sizes(path):
    """page number -> Counter of rounded /Tf size -> show-operator count."""
    out = {}
    page = 1
    while page <= 4000:
        try:
            ops = dump(path, page)
        except (IndexError, KeyError, ValueError, ZeroDivisionError):
            break
        counts = collections.Counter()
        for font, _base, sz, _tm, _td in ops:
            if font != 'TL' and sz:
                counts[round(sz, 2)] += 1
        out[page] = counts
        page += 1
    return out


def dominant(counts):
    if not counts:
        return (0.0, 0)
    return counts.most_common(1)[0]


def classify(ours, ref):
    """None when the page agrees, else (ours, ref, verdict)."""
    (da, na), (db, nb) = dominant(ours), dominant(ref)
    if abs(da - db) < 0.01 or not da or not db:
        return None
    if abs((da / db) - 1.0) <= 0.01:
        return (da, db, 'grid')

    # Does each side's dominant size appear on the other in comparable strength? If it does,
    # both renderings drew both sizes and only the split into operators differs.
    if ref.get(da, 0) >= SHARE * na and ours.get(db, 0) >= SHARE * nb:
        return (da, db, 'granularity')

    if nb < SHARE * na:
        return (da, db, 'ceiling')
    if not on_grid(db):
        return (da, db, 'metafile')
    return (da, db, 'REAL')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('sweepdir')
    ap.add_argument('--filter', default='')
    ap.add_argument('--verbose', action='store_true')
    args = ap.parse_args()

    ours_dir = os.path.join(args.sweepdir, 'ours')
    ref_dir = os.path.join(args.sweepdir, 'ref')

    verdicts = collections.Counter()
    per_doc = collections.Counter()
    pairs = collections.Counter()
    pages = docs = 0

    for name in sorted(os.listdir(ours_dir)):
        if not name.endswith('.pdf') or args.filter not in name:
            continue
        other = os.path.join(ref_dir, name)
        if not os.path.exists(other):
            continue
        try:
            a, b = sizes(os.path.join(ours_dir, name)), sizes(other)
        except Exception as exc:                       # noqa: BLE001 — a probe
            print(f'{name[:-4]}: unreadable ({exc})')
            continue
        docs += 1
        shown = False
        for p in sorted(set(a) & set(b)):
            pages += 1
            verdict = classify(a[p], b[p])
            if verdict is None:
                continue
            da, db, kind = verdict
            verdicts[kind] += 1
            if kind != 'REAL':
                continue
            per_doc[name[:-4]] += 1
            pairs[(da, db)] += 1
            if args.verbose:
                if not shown:
                    print(name[:-4])
                    shown = True
                print(f'  page {p:4d}  ours {da:7.2f} x{a[p][da]:<4d} '
                      f'ref {db:7.2f} x{b[p][db]:<4d}  ({(da / db - 1) * 100:+.2f}%)')

    print(f'\npages compared                             {pages}  over {docs} documents')
    print(f'  dominant size agrees                     '
          f'{pages - sum(verdicts.values())}')
    print(f'  differs, within 1% — the mm100 grid      {verdicts["grid"]}')
    print(f'  differs, same sizes cut up differently   {verdicts["granularity"]}')
    print(f'  differs, the reference draws far less    {verdicts["ceiling"]}   raster ceiling')
    print(f'  differs, its size is off the mm100 grid  {verdicts["metafile"]}   metafile text')
    print(f'  differs, and neither explains it         {verdicts["REAL"]}'
          f'   over {len(per_doc)} documents')

    print('\nworst documents:')
    for doc, n in per_doc.most_common(20):
        print(f'  {n:4d}  {doc}')

    print('\ncommonest (ours, ref) pairs:')
    for (x, y), n in pairs.most_common(20):
        print(f'  {n:4d}  ours {x:7.2f}  ref {y:7.2f}   ratio {x / y:.4f}')


if __name__ == '__main__':
    main()
