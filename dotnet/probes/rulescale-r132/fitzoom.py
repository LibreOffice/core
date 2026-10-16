#!/usr/bin/env python3
r"""Recover each scaled document's page transform from round 123's own banked pairs, and test
the O70 model against it WITHOUT rendering anything.

    fitzoom.py <compare-scaled.tsv.pairs> > fitzoom.tsv

Round 123 banked, for every reference text rule it could pair, the reference's drawn thickness
and centre depth and this tree's.  This tree's value is `round(p x U)` logical units for an
integer pixel count p and U = 2540/720 hundredths of a millimetre per 720 dpi pixel, so p is
recoverable from it.  The O70 claim is that the reference computes `round(p x U / z) x z` for the
page's own transform z.

**The fit has one free parameter and it is a whole percent.**  `SheetPagination.ZoomPercentage`
is an int and `ScPrintFunc::CalcZoom` bisects on one, so a Calc page's transform is one of the
91 values 10% to 100%.  Fitting that single integer per document against every rule in it --
thickness AND depth, two independent equations each -- is a test rather than a curve fit.

A document whose rules already agree does not identify z at all: every z reproduces an unscaled
rule.  `ident` counts the whole percents that score as well as the best one, so 91 means the
document says nothing about z and 1 means it pins it.
"""
import sys, collections

MM100_PER_PT = 2540.0 / 72.0
U = 2540.0 / 720.0            # hundredths of a millimetre per 720 dpi device pixel
TOL = 0.0015 * MM100_PER_PT   # the PDF channel's own floor, in hundredths of a millimetre


def main(path):
    rows = collections.defaultdict(list)
    with open(path) as fh:
        next(fh)
        for line in fh:
            doc, page, x0, rd, rw, od, ow = line.rstrip('\n').split('\t')
            rows[doc].append((float(rd), float(rw), float(od), float(ow)))

    print('doc\trules\tzpct\tident\tnow_exact\tmodel_exact\tworst_pt')
    tot = collections.Counter()
    for doc in sorted(rows):
        items = []
        for rd, rw, od, ow in rows[doc]:
            dm, wm = od * MM100_PER_PT, ow * MM100_PER_PT
            # The integer device-pixel count this tree's own value came from.
            items.append((int(round(dm / U)), rd * MM100_PER_PT,
                          int(round(wm / U)), rw * MM100_PER_PT))

        scores = []
        for zp in range(10, 101):
            z = zp / 100.0
            n = sum(1 for pd, rd, pw, rw in items
                    if abs(round(pd * U / z) * z - rd) <= TOL
                    and abs(round(pw * U / z) * z - rw) <= TOL)
            scores.append((n, zp))
        bestn = max(n for n, _ in scores)
        ident = sum(1 for n, _ in scores if n == bestn)
        zp = min(zp for n, zp in scores if n == bestn)
        z = zp / 100.0

        now = sum(1 for pd, rd, pw, rw in items
                  if abs(round(pd * U) - rd) <= TOL and abs(round(pw * U) - rw) <= TOL)
        worst = max(max(abs(round(pd * U / z) * z - rd), abs(round(pw * U / z) * z - rw))
                    for pd, rd, pw, rw in items) / MM100_PER_PT

        print('%s\t%d\t%d\t%d\t%d\t%d\t%.4f' % (doc, len(items), zp, ident, now, bestn, worst))
        tot['rules'] += len(items)
        tot['now'] += now
        tot['model'] += bestn
    print('# rules %d  now %d  model %d' % (tot['rules'], tot['now'], tot['model']),
          file=sys.stderr)


if __name__ == '__main__':
    main(sys.argv[1])
