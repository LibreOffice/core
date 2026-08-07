#!/usr/bin/env python3
"""The check that says a reused reference directory really was the same reference.

`track-ink-sweep.sh --refdir` skips re-running soffice. That is right — nothing touched the
binary — but the claim needs the evidence, and the evidence is that every reference-side
column is byte-identical between the two runs. Columns are `ours/ref` pairs, so the right of
each slash is the reference's.
"""
import sys

a_path, b_path = sys.argv[1], sys.argv[2]


def load(p):
    out = {}
    for line in open(p, encoding='utf-8'):
        f = line.rstrip('\n').split('\t')
        if len(f) >= 7:
            out[f[0]] = f
    return out


a, b = load(a_path), load(b_path)
if set(a) != set(b):
    print(f'document sets differ: {len(set(a) ^ set(b))} only in one')
    sys.exit(2)

bad = 0
for p in sorted(a):
    for col, name in ((2, 'pages'), (3, 'words'), (4, 'fonts')):
        ra = a[p][col].split('/')[-1]
        rb = b[p][col].split('/')[-1]
        if ra != rb:
            print(f'  {name}: {ra} -> {rb}   {p}')
            bad += 1

print(f'{len(a)} documents, reference-side mismatches: {bad}')
sys.exit(1 if bad else 0)
