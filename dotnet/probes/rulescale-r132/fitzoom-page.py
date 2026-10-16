#!/usr/bin/env python3
r"""The same fit as `fitzoom.py`, one whole-percent transform per PAGE rather than per document.

    fitzoom-page.py <compare-scaled.tsv.pairs> > fitzoom-page.tsv

A workbook prints each sheet at its own zoom -- `ScPrintFunc::CalcZoom` is called per range
(`printfun.cxx`:2718) -- so a document with two sheets has two transforms and one number cannot
fit both.  This bounds what the model can reach when it is given the transform the page was
actually drawn at, which is what the renderer has and this offline instrument does not.
"""
import sys, collections

MM100_PER_PT = 2540.0 / 72.0
U = 2540.0 / 720.0
TOL = 0.0015 * MM100_PER_PT


def main(path):
    rows = collections.defaultdict(list)
    with open(path) as fh:
        next(fh)
        for line in fh:
            doc, page, x0, rd, rw, od, ow = line.rstrip('\n').split('\t')
            rows[(doc, int(page))].append((float(rd), float(rw), float(od), float(ow)))

    per = collections.Counter()
    tot = collections.Counter()
    for key in sorted(rows):
        items = [(int(round(od * MM100_PER_PT / U)), rd * MM100_PER_PT,
                  int(round(ow * MM100_PER_PT / U)), rw * MM100_PER_PT)
                 for rd, rw, od, ow in rows[key]]
        bestn = max(
            sum(1 for pd, rd, pw, rw in items
                if abs(round(pd * U / (zp / 100.0)) * (zp / 100.0) - rd) <= TOL
                and abs(round(pw * U / (zp / 100.0)) * (zp / 100.0) - rw) <= TOL)
            for zp in range(10, 101))
        now = sum(1 for pd, rd, pw, rw in items
                  if abs(round(pd * U) - rd) <= TOL and abs(round(pw * U) - rw) <= TOL)
        per[(key[0], 'rules')] += len(items)
        per[(key[0], 'now')] += now
        per[(key[0], 'model')] += bestn
        tot['rules'] += len(items)
        tot['now'] += now
        tot['model'] += bestn

    print('doc\trules\tnow_exact\tmodel_exact')
    for doc in sorted({k[0] for k in per}):
        print('%s\t%d\t%d\t%d' % (doc, per[(doc, 'rules')], per[(doc, 'now')],
                                  per[(doc, 'model')]))
    print('# rules %d  now %d  model %d' % (tot['rules'], tot['now'], tot['model']),
          file=sys.stderr)


if __name__ == '__main__':
    main(sys.argv[1])
