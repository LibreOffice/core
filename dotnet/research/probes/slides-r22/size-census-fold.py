#!/usr/bin/env python3
"""Fold `size-census-doc.py`'s rows into `size-census-2.py`'s summary.

    size-census-fold.py <census.tsv>

Prints the same six lines the whole-corpus census prints, so a round's figure is comparable
with every earlier round's line for line.
"""
import collections
import sys


def main():
    verdicts = collections.Counter()
    per_doc = collections.Counter()
    pairs = collections.Counter()
    pages = 0
    docs = set()

    with open(sys.argv[1], encoding='utf-8') as fh:
        for line in fh:
            parts = line.rstrip('\n').split('\t')
            if len(parts) != 5:
                continue
            name, _p, da, db, kind = parts
            docs.add(name)
            if kind.startswith('PAGES '):
                pages += int(kind.split()[1])
                continue
            if kind.startswith('unreadable'):
                continue
            verdicts[kind] += 1
            if kind != 'REAL':
                continue
            per_doc[name] += 1
            pairs[(float(da), float(db))] += 1

    print(f'\npages compared                             {pages}  over {len(docs)} documents')
    print(f'  dominant size agrees                     {pages - sum(verdicts.values())}')
    print(f'  differs, within 1% — the mm100 grid      {verdicts["grid"]}')
    print(f'  differs, same sizes cut up differently   {verdicts["granularity"]}')
    print(f'  differs, the reference draws far less    {verdicts["ceiling"]}   raster ceiling')
    print(f'  differs, its size is off the mm100 grid  {verdicts["metafile"]}   metafile text')
    print(f'  differs, and neither explains it         {verdicts["REAL"]}'
          f'   over {len(per_doc)} documents')

    print('\nworst documents:')
    for doc, n in per_doc.most_common(20):
        print(f'  {n:4d}  {doc}')

    print('\ncommonest (ours, ref) pairs:')
    for (x, y), n in pairs.most_common(20):
        print(f'  {n:4d}  ours {x:7.2f}  ref {y:7.2f}   ratio {x / y:.4f}')


if __name__ == '__main__':
    main()
