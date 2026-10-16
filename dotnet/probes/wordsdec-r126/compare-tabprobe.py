#!/usr/bin/env python3
"""Pair our tab-probe rendering against 26.2.4.2's, row by row.

    compare-tabprobe.py <ref.tsv> <ours.tsv> <out.tsv>

Scored on horizontal COVER and on whether the tab is bridged, never on object count (C16).
"""
import sys

ref, ours, out = sys.argv[1:4]


def load(p):
    rows = {}
    with open(p) as fh:
        head = fh.readline().rstrip('\n').split('\t')
        for line in fh:
            f = line.rstrip('\n').split('\t')
            rows[f[0]] = dict(zip(head, f))
    return rows


R, O = load(ref), load(ours)
agree = {}
total = {}
with open(out, 'w') as fh:
    fh.write('label\tface\tsize\tkind\tgroup\ttabtwips\tref_cover\tours_cover\tdelta\t'
             'ref_bridged\tours_bridged\tverdict\n')
    for k in sorted(R, key=lambda k: int(k[1:])):
        r, o = R[k], O.get(k)
        if o is None or 'cover' not in r or 'cover' not in o:
            continue
        rc, oc = float(r['cover']), float(o['cover'])
        # "bridged" is the question the seat asks: does one rule reach across the tab?
        rb = 1 if (int(r['nrules']) >= 1 and float(r['cover']) > 0
                   and r['contiguous'] == '1') else 0
        ob = 1 if (int(o['nrules']) >= 1 and float(o['cover']) > 0
                   and o['contiguous'] == '1') else 0
        # a 0.25 pt allowance: the reference joins its tab stroke to the next text's stroke with
        # up to a fifth of a point of rounding, which is not a missing bridge
        ok = abs(rc - oc) <= 0.25 and rb == ob
        grp = r['group'] if r['group'] != 'span' else r['kind']
        total[grp] = total.get(grp, 0) + 1
        agree[grp] = agree.get(grp, 0) + (1 if ok else 0)
        fh.write('%s\t%s\t%s\t%s\t%s\t%s\t%.2f\t%.2f\t%.2f\t%d\t%d\t%s\n'
                 % (k, r['face'], r['size'], r['kind'], r['group'], r['tabtwips'],
                    rc, oc, oc - rc, rb, ob, 'agree' if ok else 'DIFFER'))

for g in sorted(total):
    print('%-8s %3d of %3d agree' % (g, agree[g], total[g]))
print('%-8s %3d of %3d agree' % ('TOTAL', sum(agree.values()), sum(total.values())))
