"""Score a sheets-track sweep against the banked gate at 2f4709c08.

Refuses to print a summary unless every banked sheets path found a row, because a sweep that
lost a worker reports a smaller total and a plausible-looking verdict list.
"""
import sys

def rows(path):
    out = {}
    for line in open(path, encoding='utf-8'):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] == 'path':
            continue
        out[f[0]] = f
    return out

banked = {k: v for k, v in rows('/home/user/gate-2f47/parity.tsv').items() if k.startswith('sheets/')}
after = {k: v for k, v in rows(sys.argv[1]).items() if k.startswith('sheets/')}

missing = sorted(set(banked) - set(after))
extra = sorted(set(after) - set(banked))
print(f'banked sheets rows: {len(banked)}   swept: {len(after)}')
if missing:
    print(f'MISSING {len(missing)} banked paths - the sweep is incomplete, do not score it:')
    for m in missing[:20]:
        print('  ', m)
    sys.exit(1)
if extra:
    print(f'note: {len(extra)} paths in the sweep and not in the bank (case aliases?):')
    for m in extra[:10]:
        print('  ', m)

moved = []
numbers = []
for path, b in sorted(banked.items()):
    a = after[path]
    if b[6] != a[6]:
        moved.append((path, b[6], a[6]))
    elif b[2:6] != a[2:6] or (len(b) > 8 and len(a) > 8 and b[8] != a[8]):
        numbers.append((path, b[2:], a[2:]))

from collections import Counter
print()
print('banked verdicts :', dict(Counter(v[6] for v in banked.values())))
print('sweep  verdicts :', dict(Counter(v[6] for v in after.values())))
print()
print(f'verdicts moved: {len(moved)}')
for p, was, now in moved:
    arrow = 'IMPROVED' if now == 'match' else ('REGRESSED' if was == 'match' else 'changed')
    print(f'  {arrow:9s} {was:12s} -> {now:12s}  {p}')
print()
print(f'numbers moved without the verdict: {len(numbers)}')
for p, was, now in numbers:
    print(f'  {p}')
    print(f'      banked {was}')
    print(f'      sweep  {now}')
