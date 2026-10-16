#!/usr/bin/env python3
"""Compare two batch-check sweeps of the same corpus, the way the rulebook says to.

    compare.py BEFORE/rows.tsv AFTER/rows.tsv

A gate run under CPU contention undercounts on the *reference* side, so the totals of two
sweeps are not comparable on their own: every row that failed on either side in either run is
excluded before anything is counted, and the excluded ones are named.
"""
import sys


def read(path):
    rows = {}
    with open(path, encoding='utf8') as handle:
        for line in handle:
            parts = line.rstrip('\n').split('\t')
            if len(parts) < 9:
                continue
            rows[parts[0]] = parts
    return rows


def main(before_path, after_path):
    before = read(before_path)
    after = read(after_path)

    shared = sorted(set(before) & set(after))
    only_before = sorted(set(before) - set(after))
    only_after = sorted(set(after) - set(before))

    bad = {k for k in shared if 'failed' in before[k][6] or 'failed' in after[k][6]}

    gained, lost, moved = [], [], []
    for key in shared:
        if key in bad:
            continue
        b, a = before[key][6], after[key][6]
        if b == a:
            if before[key][2:] != after[key][2:]:
                moved.append((key, before[key][2], after[key][2], before[key][8], after[key][8]))
            continue
        (gained if a == 'match' else lost).append(
            (key, b, a, before[key][2], after[key][2], before[key][8], after[key][8]))

    comparable = [k for k in shared if k not in bad]
    print(f'rows in both       {len(shared)}')
    print(f'excluded (failed)  {len(bad)}')
    for key in sorted(bad):
        print(f'    {key}\t{before[key][6]}\t{after[key][6]}')
    if only_before:
        print(f'only in before     {len(only_before)}')
    if only_after:
        print(f'only in after      {len(only_after)}')
    print(f'comparable         {len(comparable)}')
    print(f'  match before     {sum(1 for k in comparable if before[k][6] == "match")}')
    print(f'  match after      {sum(1 for k in comparable if after[k][6] == "match")}')
    print()
    print(f'GAINED {len(gained)}')
    for row in gained:
        print('    %s\t%s -> %s\tpages %s -> %s\tglyphs %s -> %s' % row)
    print(f'LOST {len(lost)}')
    for row in lost:
        print('    %s\t%s -> %s\tpages %s -> %s\tglyphs %s -> %s' % row)
    print(f'MOVED-BUT-SAME-VERDICT {len(moved)}')
    for row in moved:
        print('    %s\tpages %s -> %s\tglyphs %s -> %s' % row)


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
