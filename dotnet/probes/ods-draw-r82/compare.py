#!/usr/bin/env python3
"""Diff two `sweep-ours.sh` row files: verdict counts, then every row that moved.

    compare.py <before.tsv> <after.tsv>

The columns are `sweep-ours.sh`'s own, which are `batch-check.sh`'s: path, ext, pages ours/ref,
words, fonts, unembedded, verdict, rawwords, glyphs.
"""
import collections
import sys


def rows(path):
    answer = {}
    for line in open(path, encoding='utf-8'):
        parts = line.rstrip('\n').split('\t')
        if len(parts) < 9:
            continue
        answer[parts[0]] = parts
    return answer


def main():
    before, after = rows(sys.argv[1]), rows(sys.argv[2])

    for name, table in (('before', before), ('after', after)):
        counts = collections.Counter(row[6] for row in table.values())
        print(name, dict(sorted(counts.items())), 'TOTAL', len(table))

    print()
    gained, lost, moved = [], [], []
    for key in sorted(set(before) | set(after)):
        b, a = before.get(key), after.get(key)
        if b is None or a is None:
            print('ONE SIDE ONLY', key)
            continue
        if b[6] == a[6] and b[2] == a[2] and b[8] == a[8]:
            continue
        line = (f'{key.split("/")[-1]}\t{b[6]} -> {a[6]}\t'
                f'pages {b[2]} -> {a[2]}\tglyphs {b[8]} -> {a[8]}')
        if b[6] != 'match' and a[6] == 'match':
            gained.append(line)
        elif b[6] == 'match' and a[6] != 'match':
            lost.append(line)
        else:
            moved.append(line)

    print(f'gained {len(gained)}, lost {len(lost)}, moved without a verdict change {len(moved)}')
    for title, group in (('GAINED', gained), ('LOST', lost), ('MOVED', moved)):
        if group:
            print(f'\n== {title}')
            print('\n'.join(group))


if __name__ == '__main__':
    main()
