#!/usr/bin/env python3
"""Both ink columns and the major-page count per document, from a sweep's cmp/*.txt.

ink.tsv carries only the signed sum. The unsigned one is the column that ranks the track —
a signed sum lets a deficit cancel a surplus, so filling the deficit reads as a regression.

    ink-columns.py <sweepdir> [<sweepdir> ...]      totals per sweep
    ink-columns.py --per-document <a> <b>           every document that moved, a -> b
"""
import os
import sys


def read(sweep):
    """{document id: (signed, unsigned, major, pages)}"""
    out = {}
    directory = os.path.join(sweep, 'cmp')
    if not os.path.isdir(directory):
        return out
    for name in sorted(os.listdir(directory)):
        if not name.endswith('.txt'):
            continue
        signed = unsigned = 0.0
        major = pages = 0
        for line in open(os.path.join(directory, name), encoding='utf-8', errors='replace'):
            parts = line.rstrip('\n').split('\t')
            if len(parts) < 6 or not parts[0].isdigit():
                continue
            try:
                signed += float(parts[2])
                unsigned += float(parts[3])
            except ValueError:
                continue
            pages += 1
            if parts[5].strip() == 'MAJOR':
                major += 1
        out[name[:-4]] = (signed, unsigned, major, pages)
    return out


def total(rows):
    return (sum(r[0] for r in rows.values()),
            sum(r[1] for r in rows.values()),
            sum(r[2] for r in rows.values()),
            len(rows))


def main():
    args = sys.argv[1:]
    per_document = args and args[0] == '--per-document'
    if per_document:
        args = args[1:]

    tables = [(a, read(a)) for a in args]
    for name, rows in tables:
        s, u, m, n = total(rows)
        print(f'{os.path.basename(name):24s} signed {s:9.2f}  |ink| {u:9.2f}  '
              f'major {m:5d}  over {n} documents')

    if per_document and len(tables) == 2:
        (_, before), (_, after) = tables
        moved = []
        for key in sorted(set(before) | set(after)):
            b = before.get(key)
            a = after.get(key)
            if b is None or a is None:
                print(f'  MISSING {key}: {"before" if b is None else "after"}')
                continue
            if abs(a[1] - b[1]) < 0.005 and a[2] == b[2]:
                continue
            moved.append((a[1] - b[1], key, b, a))
        moved.sort()
        print(f'\n{len(moved)} documents moved on |ink| or major pages')
        better = sum(1 for d, *_ in moved if d < 0)
        won = -sum(d for d, *_ in moved if d < 0)
        lost = sum(d for d, *_ in moved if d > 0)
        print(f'  {better} better, {len(moved) - better} worse; '
              f'{won:.2f} won against {lost:.2f} lost')
        for delta, key, b, a in moved:
            print(f'  {delta:+8.2f}  |ink| {b[1]:8.2f} -> {a[1]:8.2f}   '
                  f'major {b[2]:3d} -> {a[2]:3d}   {key}')


if __name__ == '__main__':
    main()
