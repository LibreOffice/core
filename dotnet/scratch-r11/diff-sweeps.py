#!/usr/bin/env python3
"""Diff two batch-check sweeps document by document."""
import sys, os

def load(p):
    d = {}
    for line in open(p, encoding='utf8'):
        f = line.rstrip('\n').split('\t')
        if len(f) < 7:
            continue
        d[f[0]] = f
    return d

a, b = load(sys.argv[1]), load(sys.argv[2])
moved = 0
for k in sorted(set(a) | set(b)):
    ra, rb = a.get(k), b.get(k)
    if ra is None or rb is None:
        print(f"  ONLY IN ONE: {k}")
        continue
    if ra[2:7] == rb[2:7]:
        continue
    moved += 1
    verdict = ''
    if ra[6] != rb[6]:
        verdict = f"   {ra[6]} -> {rb[6]}"
    print(f"  pg {ra[2]:>10} -> {rb[2]:<10} w {ra[3]:>14} -> {rb[3]:<14}{verdict}  {os.path.basename(k)[:58]}")
print(f"\n{moved} documents moved")
