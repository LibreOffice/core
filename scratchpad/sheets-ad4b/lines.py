#!/usr/bin/env python3
"""Row heights per sheet turned into line counts (one line = 18 px = 268.7 twips)."""
import sys, collections

d = collections.defaultdict(dict)
for line in open(sys.argv[1]):
    p = line.rstrip('\n').split('\t')
    if len(p) < 4 or p[3] == '-': continue
    d[p[0]][int(p[1])] = float(p[3])


def count(h):
    return round((h - 29.5) / 268.7)


def key(s):
    t = ''.join(c for c in s if c.isdigit() or c == '.')
    try: return float(t)
    except ValueError: return 0.0


for k in sorted(d, key=key):
    print(k, [count(d[k][i]) for i in sorted(d[k])])
