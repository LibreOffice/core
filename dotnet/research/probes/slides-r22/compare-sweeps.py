#!/usr/bin/env python3
"""before/after on the slides track: verdicts, |ink|%, and which documents moved."""
import sys, collections

def rows(path):
    out = {}
    for line in open(path, encoding='utf-8', errors='replace'):
        f = line.rstrip('\n').split('\t')
        if len(f) >= 7:
            out[f[0]] = f
    return out

def ink(path, correct):
    """path -> (signed ink%, |ink|%, major pages).

    `correct` undoes the aggregation defect the inherited sweep script carried: its awk
    matched the tool's own trailing summary line, "N pages, M with major differences", whose
    third field is the major-page count, so every document's *signed* column was inflated by
    exactly one per major page. The unsigned column is untouched — its fourth field there is
    "with", which is not a number. Subtracting the major count is exact, not an estimate.
    """
    out = {}
    for line in open(path, encoding='utf-8', errors='replace'):
        f = line.rstrip('\n').split('\t')
        if len(f) >= 6:
            signed, unsigned, major = float(f[2]), float(f[3]), int(f[4])
            out[f[0]] = (signed - (major if correct else 0), unsigned, major)
    return out

a, b = sys.argv[1], sys.argv[2]
ra, rb = rows(a + '/rows.tsv'), rows(b + '/rows.tsv')
ia = ink(a + '/ink.tsv', correct='--fix-a' in sys.argv)
ib = ink(b + '/ink.tsv', correct='--fix-b' in sys.argv)

print(f'rows        {len(ra)} -> {len(rb)}')
ma = sum(1 for f in ra.values() if f[6] == 'match')
mb = sum(1 for f in rb.values() if f[6] == 'match')
print(f'word gate   {ma}/{len(ra)} -> {mb}/{len(rb)}')

def agg(d):
    s = sum(v[0] for v in d.values()); t = sum(v[1] for v in d.values())
    m = sum(v[2] for v in d.values())
    return s, t, m
sa, ta, mja = agg(ia); sb_, tb, mjb = agg(ib)
print(f'ink%        {sa:.2f} -> {sb_:.2f}')
print(f'|ink|%      {ta:.2f} -> {tb:.2f}')
print(f'major pages {mja} -> {mjb}   over {len(ia)} / {len(ib)} documents')
assert ta + 0.005 >= abs(sa) and tb + 0.005 >= abs(sb_), 'aggregate invariant violated'

changed = [(k, ib[k][1] - ia[k][1]) for k in ia if k in ib and abs(ib[k][1] - ia[k][1]) > 0.005]
better = [c for c in changed if c[1] < 0]; worse = [c for c in changed if c[1] > 0]
print(f'\n|ink|% moved on {len(changed)} documents: '
      f'{len(better)} better ({sum(c[1] for c in better):.2f}), '
      f'{len(worse)} worse (+{sum(c[1] for c in worse):.2f})')
for k, d in sorted(changed, key=lambda c: c[1])[:15]:
    print(f'  {d:+8.2f}  {k}')
print('  ...')
for k, d in sorted(changed, key=lambda c: -c[1])[:8]:
    print(f'  {d:+8.2f}  {k}')

flips = [(k, ra[k][6], rb[k][6]) for k in ra if k in rb and ra[k][6] != rb[k][6]]
print(f'\nverdicts changed: {len(flips)}')
for k, x, y in flips:
    print(f'  {x} -> {y}   {k}')

# per batch
def batch(k): return k.split('/')[1] if '/' in k else '?'
pa = collections.Counter(); pb = collections.Counter(); tot = collections.Counter()
for k, f in ra.items():
    tot[batch(k)] += 1
    if f[6] == 'match': pa[batch(k)] += 1
for k, f in rb.items():
    if f[6] == 'match': pb[batch(k)] += 1
print('\nper batch (before -> after / total)')
for bch in sorted(tot):
    flag = '' if pa[bch] == pb[bch] else '   <-- moved'
    print(f'  {bch}  {pa[bch]:2d} -> {pb[bch]:2d} / {tot[bch]:2d}{flag}')
