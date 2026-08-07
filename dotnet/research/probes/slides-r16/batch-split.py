#!/usr/bin/env python3
"""Per-batch match counts from two rows.tsv, so a flat total cannot hide a swap."""
import collections
import sys


def load(p):
    out = {}
    for line in open(p, encoding='utf-8'):
        f = line.rstrip('\n').split('\t')
        if len(f) >= 7:
            out[f[0]] = f
    return out


a, b = load(sys.argv[1]), load(sys.argv[2])
ca, cb, tot = collections.Counter(), collections.Counter(), collections.Counter()
for p, f in a.items():
    batch = p.split('/')[1]
    tot[batch] += 1
    if f[6] == 'match':
        ca[batch] += 1
for p, f in b.items():
    if f[6] == 'match':
        cb[batch := p.split('/')[1]] += 1

print(f'{"batch":10} {"before":>8} {"after":>8}  {"of":>3}')
for batch in sorted(tot):
    flag = '' if ca[batch] == cb[batch] else '   <-- moved'
    print(f'{batch:10} {ca[batch]:8d} {cb[batch]:8d}  {tot[batch]:3d}{flag}')
print(f'{"TOTAL":10} {sum(ca.values()):8d} {sum(cb.values()):8d}  {sum(tot.values()):3d}')

print('\nverdict changes:')
for p in sorted(set(a) | set(b)):
    x, y = a.get(p, ['?'] * 7), b.get(p, ['?'] * 7)
    if x[6] != y[6]:
        print(f'  {x[6]} -> {y[6]}   {p}')
        print(f'      pages {x[2]} -> {y[2]}   words {x[3]} -> {y[3]}   fonts {x[4]} -> {y[4]}')
