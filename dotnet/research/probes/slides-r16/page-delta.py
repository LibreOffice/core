#!/usr/bin/env python3
"""Per-page |ink|% for one document across two sweeps, largest movement first."""
import os
import sys

a_dir, b_dir, key = sys.argv[1], sys.argv[2], sys.argv[3]


def pages(outdir):
    path = os.path.join(outdir, 'cmp', key + '.txt')
    out = {}
    for line in open(path, encoding='utf-8'):
        f = line.rstrip('\n').split('\t')
        if len(f) >= 6 and f[0].isdigit():
            out[int(f[0])] = (float(f[2]), float(f[3]), f[5] if len(f) > 5 else '')
    return out


a, b = pages(a_dir), pages(b_dir)
rows = []
for p in sorted(set(a) | set(b)):
    x = a.get(p, (0.0, 0.0, ''))
    y = b.get(p, (0.0, 0.0, ''))
    rows.append((y[1] - x[1], p, x, y))

print(f'{"page":>5} {"d|ink|":>8} {"before":>8} {"after":>8}   verdict')
for d, p, x, y in sorted(rows, reverse=True):
    if abs(d) < 0.005:
        continue
    print(f'{p:5d} {d:+8.2f} {x[1]:8.2f} {y[1]:8.2f}   {x[2]} -> {y[2]}')

print(f'\ntotal |ink|%  {sum(v[1] for v in a.values()):.2f} -> {sum(v[1] for v in b.values()):.2f}')
print(f'major pages   {sum(1 for v in a.values() if "MAJOR" in v[2])} -> '
      f'{sum(1 for v in b.values() if "MAJOR" in v[2])}')
