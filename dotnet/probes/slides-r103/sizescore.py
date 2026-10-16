#!/usr/bin/env python3
"""O15's statistic, scored: the per-page dominant drawn text size against 26.2.4.2's.

    sizescore.py <ref.tsv> <base.tsv> <head.tsv>

Each file is `sizes.py`'s output -- <id>\\t<page>\\t<size>\\t<alphanumerics at that size>.
Prints the register's own summary for both legs and lists every page still differing, so the
list is in the record rather than only the count.
"""
import collections, sys


def load(path):
    out = {}
    with open(path) as fh:
        for line in fh:
            ident, page, size, n = line.rstrip('\n').split('\t')
            out[(ident, int(page))] = (float(size), int(n))
    return out


def score(ref, leg):
    keys = [k for k in ref if k in leg]
    bad = [k for k in keys if abs(ref[k][0] - leg[k][0]) > 0.15]
    total = sum(abs(ref[k][0] - leg[k][0]) for k in keys)
    same = [k for k in bad if ref[k][1] == leg[k][1]]
    larger = [k for k in same if leg[k][0] > ref[k][0]]
    return keys, bad, total, same, larger


def main(refp, basep, headp):
    ref, base, head = load(refp), load(basep), load(headp)
    print(f'pages scored on all three legs: '
          f'{len([k for k in ref if k in base and k in head])}')
    for name, leg in (('base', base), ('head', head)):
        keys, bad, total, same, larger = score(ref, leg)
        docs = len({k[0] for k in bad})
        print(f'{name}\tpages differing >0.15pt {len(bad):4d}\tdocuments {docs:3d}\t'
              f'total |size error| {total:8.2f}\tsame-alnum {len(same):3d}\t'
              f'of those we draw larger {len(larger):3d}')
    fixed = [k for k in ref if k in base and k in head
             and abs(ref[k][0] - base[k][0]) > 0.15 and abs(ref[k][0] - head[k][0]) <= 0.15]
    broke = [k for k in ref if k in base and k in head
             and abs(ref[k][0] - base[k][0]) <= 0.15 and abs(ref[k][0] - head[k][0]) > 0.15]
    moved = [k for k in base if k in head and base[k][0] != head[k][0]]
    print(f'fixed: {len(fixed)}   newly wrong: {len(broke)}   '
          f'pages whose dominant size moved between base and head: {len(moved)}')
    for label, rows in (('fixed', fixed), ('newly wrong', broke)):
        for k in sorted(rows):
            print(f'  {label}\t{k[0][:52]:52} {k[1]:4d}  ref {ref[k][0]:6}  '
                  f'base {base[k][0]:6}  head {head[k][0]:6}')
    print('\nstill differing at head:')
    print(f'   {"document":52} {"page":>4} {"ref":>7} {"ours":>7} {"ref alnum":>10} {"our alnum":>10}')
    for k in sorted(score(ref, head)[1]):
        print(f'   {k[0][:52]:52} {k[1]:4d} {ref[k][0]:7} {head[k][0]:7} '
              f'{ref[k][1]:10} {head[k][1]:10}')
    per = collections.Counter(k[0] for k in score(ref, head)[1])
    print('\nper document:', dict(per))


if __name__ == '__main__':
    main(*sys.argv[1:4])
