#!/usr/bin/env python3
"""Which identities' renderings differ between two sweeps' hash tables."""
import sys


def read(path):
    rows = {}
    with open(path, encoding='utf-8') as fh:
        fh.readline()
        for line in fh:
            p = line.rstrip('\n').split('\t')
            if len(p) >= 4:
                rows[p[0]] = (p[1], p[2], p[3])
    return rows


a, b = read(sys.argv[1]), read(sys.argv[2])
moved = []
for ident, (rel, status, digest) in sorted(a.items()):
    if ident not in b:
        print(f'MISSING\t{ident}')
        continue
    rel2, status2, digest2 = b[ident]
    if status != 'ok' or status2 != 'ok':
        print(f'STATUS\t{ident}\t{status}\t{status2}')
        continue
    if digest != digest2:
        moved.append((ident, rel))
print(f'compared {len(a)} of {len(b)}; differ {len(moved)}')
for ident, rel in moved:
    print(f'MOVED\t{ident}\t{rel}')
