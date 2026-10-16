#!/usr/bin/env python3
"""How a change is distributed across a track, not just what it totals.

A net figure of +2.00 over 1751 can be one document going badly wrong or a hundred documents
drifting. Those want opposite responses, and the total cannot tell them apart.
"""
import sys

sys.path.insert(0, '/home/user/libreoffice-core/.claude/worktrees/agent-a5e04a5cda6ded500/dotnet/research/probes/slides-r15')
from importlib.machinery import SourceFileLoader

mod = SourceFileLoader('ink_columns', '/home/user/libreoffice-core/.claude/worktrees/'
                       'agent-a5e04a5cda6ded500/dotnet/research/probes/slides-r15/'
                       'ink-columns.py').load_module()

before = mod.totals(sys.argv[1])
after = mod.totals(sys.argv[2])

rows = []
for p in sorted(set(before) | set(after)):
    b = before.get(p, (0.0, 0.0, 0, 0))
    a = after.get(p, (0.0, 0.0, 0, 0))
    rows.append((a[1] - b[1], a[2] - b[2], b[1], a[1], p))

better = [r for r in rows if r[0] < -0.005]
worse = [r for r in rows if r[0] > 0.005]
flat = [r for r in rows if abs(r[0]) <= 0.005]

print(f'documents: {len(rows)}   better {len(better)}   worse {len(worse)}   unchanged {len(flat)}')
print(f'|ink|% won on the better ones: {-sum(r[0] for r in better):8.2f}')
print(f'|ink|% lost on the worse ones: {sum(r[0] for r in worse):8.2f}')
print(f'net: {sum(r[0] for r in rows):+.2f}')
print(f'major pages: {sum(r[1] for r in rows):+d}')

print('\nten largest gains:')
for d, dm, b, a, p in sorted(rows)[:10]:
    print(f'  {d:+8.2f}  {b:7.2f} -> {a:7.2f}  major {dm:+d}  {p}')
print('\nten largest losses:')
for d, dm, b, a, p in sorted(rows, reverse=True)[:10]:
    print(f'  {d:+8.2f}  {b:7.2f} -> {a:7.2f}  major {dm:+d}  {p}')

n = len(worse)
print(f'\nwithout the single worst document the net would be '
      f'{sum(r[0] for r in rows) - max(r[0] for r in rows):+.2f}' if n else '')
