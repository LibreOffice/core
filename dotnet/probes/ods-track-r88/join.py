#!/usr/bin/env python3
"""Split an .ods gate row file by whether the same document's ORIGINAL (.xlsx/.xls/.xlsm)
row passes its own gate.  An .ods row that fails where its original passes is the ODF
reader's; one that fails in both is shared layout.

    join.py <ods rows.tsv> <original rows.tsv>
"""
import sys, os, collections

def load(path):
    rows = {}
    for line in open(path, encoding='utf-8'):
        f = line.rstrip('\n').split('\t')
        if len(f) < 9: continue
        stem = os.path.splitext(os.path.basename(f[0]))[0]
        rows[stem] = f
    return rows

ods = load(sys.argv[1])
orig = load(sys.argv[2])

def pct(f):
    try:
        a, b = f[8].split('/'); a = float(a); b = float(b)
        return (a - b) / b * 100 if b else 0.0
    except Exception:
        return 0.0

groups = collections.defaultdict(list)
for stem, f in sorted(ods.items()):
    v = f[6]
    if v == 'match': continue
    o = orig.get(stem)
    ov = o[6] if o else 'no-original'
    key = ('ODF-READER' if ov == 'match' else 'SHARED' if o else 'NO-ORIGINAL')
    groups[key].append((v, f[2], f[8], pct(f), ov, (o[2] if o else '-'), stem))

for key in ('ODF-READER', 'SHARED', 'NO-ORIGINAL'):
    rows = groups[key]
    print(f'== {key}: {len(rows)}')
    for v, pg, gl, p, ov, opg, stem in sorted(rows, key=lambda r: (r[0], -abs(r[3]))):
        print(f'  {v:<12} ods pages {pg:<10} glyphs {gl:<16} {p:+7.2f}%   orig {ov:<12} {opg:<10} {stem[:70]}')
    print()
