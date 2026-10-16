#!/usr/bin/env python3
"""Resolve the corpus paths of the documents named on stdin, one per line."""
import sys, pathlib
root = pathlib.Path('/home/user/sample-files')
index = {}
for q in root.rglob('*'):
    if q.is_file(): index.setdefault(q.name, q)
for line in sys.stdin:
    n = line.strip()
    if not n: continue
    p = index.get(n)
    print(p if p else f'MISSING\t{n}', file=sys.stdout if p else sys.stderr)
