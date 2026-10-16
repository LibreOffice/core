#!/usr/bin/env python3
"""Diff two sweep legs and classify every mover by track and format.

The document list is `MANIFEST.tsv`'s own paths plus the two converted ODF columns, never a `find`
total: the case-insensitive mount materialises alias entries when a tool resolves a document, so a
`find` count drifts upwards on its own and a sweep TOTAL is not comparable with the same sweep's
TOTAL an hour earlier.
"""
import collections
import csv
import hashlib
import pathlib

ROOT = pathlib.Path('/home/user/charscale-r150-sweep')
CORPUS = '/home/user/sample-files'


def catalogue():
    known = {}
    with open(CORPUS + '/MANIFEST.tsv') as handle:
        for row in csv.DictReader(handle, delimiter='\t'):
            if not row['path']:
                continue
            full = CORPUS + '/' + row['path']
            known[hashlib.md5(full.encode()).hexdigest()[:12]] = (row['family'], row['ext'])
    for column in ('rtf', 'odt'):
        for path in sorted(pathlib.Path('/home/user/corpus-odf', column).glob('*.' + column)):
            known[hashlib.md5(str(path).encode()).hexdigest()[:12]] = (column + '-column', column)
    return known


def main():
    known = catalogue()
    base, head = ROOT / 'base', ROOT / 'head'
    moved, total = collections.Counter(), collections.Counter()
    movers = []

    for folder in sorted(p.name for p in base.iterdir() if p.is_dir()):
        kind = known.get(folder, ('UNKNOWN', '?'))
        total[kind] += 1
        a = sorted((base / folder).glob('*.pdf'))
        b = sorted((head / folder).glob('*.pdf'))
        if [p.name for p in a] != [p.name for p in b]:
            moved[kind] += 1
            movers.append(folder)
            continue
        if any(x.read_bytes() != y.read_bytes() for x, y in zip(a, b)):
            moved[kind] += 1
            movers.append(folder)

    print('%-14s %-6s %7s %7s' % ('family', 'ext', 'moved', 'of'))
    for kind in sorted(total):
        print('%-14s %-6s %7d %7d' % (kind[0], kind[1], moved[kind], total[kind]))
    print()
    print('total %d of %d moved, %d byte-identical'
          % (sum(moved.values()), sum(total.values()), sum(total.values()) - sum(moved.values())))
    pathlib.Path(__file__).with_name('movers.txt').write_text('\n'.join(movers) + '\n')


if __name__ == '__main__':
    main()
