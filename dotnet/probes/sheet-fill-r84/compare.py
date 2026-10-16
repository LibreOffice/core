#!/usr/bin/env python3
"""Compare two sweep row files: verdict counts, and every row whose verdict or counts moved.

    compare.py <before.tsv> <after.tsv> [--exclude-failed]

`--exclude-failed` applies `dotnet/CLAUDE.md`'s own recipe before comparing anything: drop
every document that failed on either side in *either* run, then compare what is left. A row
that is `match` in one run and `ref-failed` in the other is evidence about the box, not about
the tree, and two sweeps must never be compared on their match totals without it.

Column 7 is the verdict, column 9 is `glyphs` as `ours/ref` — which is what the band applies
to — and column 4 is `words`, a token count kept for history.
"""
import collections
import sys


def load(path):
    rows = {}
    for line in open(path):
        f = line.rstrip('\n').split('\t')
        if len(f) >= 9:
            rows[f[0]] = f
    return rows


def main(before, after, exclude=False):
    a, b = load(before), load(after)
    keys = sorted(set(a) & set(b))
    if exclude:
        bad = {k for k in keys if 'failed' in a[k][6] or 'failed' in b[k][6]}
        keys = [k for k in keys if k not in bad]
        print(f'{len(bad)} rows failed on one side in one run and are excluded')

    ca = collections.Counter(a[k][6] for k in keys)
    cb = collections.Counter(b[k][6] for k in keys)
    print(f'{len(keys)} comparable rows')
    for v in sorted(set(ca) | set(cb)):
        print(f'  {v:16s} {ca[v]:4d} -> {cb[v]:4d}')

    moved = [k for k in keys if a[k][6] != b[k][6]]
    print(f'\n{len(moved)} verdicts moved')
    for k in moved:
        print(f'  {a[k][6]:14s} -> {b[k][6]:14s} pages {b[k][2]:10s} glyphs {b[k][8]:16s} {k}')

    counts = [k for k in keys if a[k][2] != b[k][2] or a[k][8] != b[k][8]]
    before_sum = after_sum = 0
    for k in counts:
        try:
            ob, rb = (int(x) for x in a[k][8].split('/'))
            oa, ra = (int(x) for x in b[k][8].split('/'))
        except ValueError:
            continue
        before_sum += abs(ob - rb)
        after_sum += abs(oa - ra)
    print(f'\n{len(counts)} rows changed a page or glyph count; '
          f'sum |ours-ref| glyphs over them {before_sum} -> {after_sum}')
    for k in counts:
        print(f'  pages {a[k][2]}->{b[k][2]:12s} glyphs {a[k][8]}->{b[k][8]:20s} {k}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], '--exclude-failed' in sys.argv)
