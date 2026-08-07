#!/usr/bin/env python3
"""Which tokens one extraction has and the other does not, per page."""
import collections
import sys

a = open(sys.argv[1], encoding='utf-8', errors='replace').read()
b = open(sys.argv[2], encoding='utf-8', errors='replace').read()

for page, (pa, pb) in enumerate(zip(a.split('\f'), b.split('\f')), 1):
    ta = collections.Counter(pa.split())
    tb = collections.Counter(pb.split())
    only_a = ta - tb
    only_b = tb - ta
    if not only_a and not only_b:
        continue
    print(f'--- page {page}   {sum(ta.values())} vs {sum(tb.values())} ---')
    if only_a:
        print(f'  only in {sys.argv[1].rsplit("/", 1)[-1]}: '
              f'{" ".join(sorted(only_a.elements()))[:400]}')
    if only_b:
        print(f'  only in {sys.argv[2].rsplit("/", 1)[-1]}: '
              f'{" ".join(sorted(only_b.elements()))[:400]}')
