#!/usr/bin/env python3
"""Per-document change between two batch-check sweeps."""
import sys


def load(path):
    d = {}
    for line in open(path):
        p = line.rstrip('\n').split('\t')
        if len(p) < 7: continue
        d[p[0]] = p
    return d


a, b = load(sys.argv[1]), load(sys.argv[2])
better, worse, same = [], [], 0
for k in sorted(set(a) & set(b)):
    pa, pb = a[k], b[k]
    if pa[2] == pb[2] and pa[6] == pb[6]:
        same += 1
        continue
    try:
        da = abs(int(pa[2].split('/')[0]) - int(pa[2].split('/')[1]))
        db = abs(int(pb[2].split('/')[0]) - int(pb[2].split('/')[1]))
    except Exception:
        da = db = 0
    row = '%-72s %s -> %s   %s -> %s' % (k, pa[2], pb[2], pa[6], pb[6])
    (better if db < da or (db == da and pb[6] == 'match') else worse).append(row)

print('unchanged %d' % same)
print('\n-- improved (%d)' % len(better))
for r in better: print(r)
print('\n-- worse (%d)' % len(worse))
for r in worse: print(r)
