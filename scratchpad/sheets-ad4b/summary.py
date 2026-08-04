#!/usr/bin/env python3
"""Match count, page error and page-exact count from a batch-check rows.tsv."""
import sys

rows = []
for line in open(sys.argv[1]):
    p = line.rstrip('\n').split('\t')
    if len(p) < 7: continue
    rows.append(p)

match = sum(1 for p in rows if p[6] == 'match')
err = 0
exact = 0
bad = []
for p in rows:
    try:
        ours, ref = p[2].split('/')
        o, r = int(ours), int(ref)
    except Exception:
        bad.append(p[0]); continue
    d = abs(o - r)
    err += d
    if d == 0: exact += 1
print('documents %d  match %d  page-error %d  page-exact %d  unparsed %d'
      % (len(rows), match, err, exact, len(bad)))
if len(sys.argv) > 2:
    worst = sorted(
        (p for p in rows if '/' in p[2]),
        key=lambda p: -abs(int(p[2].split('/')[0]) - int(p[2].split('/')[1])))
    for p in worst[:int(sys.argv[2])]:
        o, r = p[2].split('/')
        print('%+6d  %-70s pages %s words %s %s' % (int(o) - int(r), p[0], p[2], p[3], p[6]))
