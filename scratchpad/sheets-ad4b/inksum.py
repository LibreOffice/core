#!/usr/bin/env python3
"""Summarise an ink.sh run: totals, worst documents, and what the hints say."""
import sys, collections, re, os

path = sys.argv[1]
rows = []
for line in open(path):
    p = line.rstrip('\n').split('\t')
    if len(p) < 3: continue
    rows.append((p[0], int(p[1]), int(p[2])))

with_major = [r for r in rows if r[2] > 0]
print('documents compared %d, with at least one MAJOR page %d, MAJOR pages %d'
      % (len(rows), len(with_major), sum(r[2] for r in with_major)))
print()
for name, pages, major in sorted(with_major, key=lambda r: -r[2])[:20]:
    print('%4d/%-5d %s' % (major, pages, name))

# what the hints say, across every MAJOR page of every document
outdir = os.path.dirname(path)
hints = collections.Counter()
for name, _, major in with_major:
    f = os.path.join(outdir, name + '.txt')
    if not os.path.exists(f): continue
    text = open(f, errors='replace').read()
    for m in re.finditer(r'^\s{4,}(\S.*?)(?: \(\d|\s*$)', text, re.M):
        h = m.group(1).strip()
        if h.startswith(('page', '---')): continue
        hints[re.sub(r'^[a-z\- ]+: ', '', h)[:70]] += 1
print()
for h, n in hints.most_common(12):
    print('%5d  %s' % (n, h))
