#!/usr/bin/env python3
"""Diff two legs of the words sweep and tag each mover by what round 147 could reach in it.

The tag comes from `probes/tablerow-r146/census-pdf.tsv`, which counted, in **26.2.4.2's own
rendering** of each document, how many horizontal boundaries carry bands of more than one width.
That is the population this change touches and nothing else is: a boundary whose columns agree
is drawn in exactly the same place before and after, by construction.

Using the reference's ink rather than the markup is deliberate. A `w:tcBorders` census says what
a document *states*; it cannot say whether the two facing edges resolve to different widths at a
boundary that is actually drawn, which is the only thing that moves.
"""
import collections
import csv
import os
import sys

CORPUS = '/home/user/sample-files'
HERE = os.path.dirname(os.path.abspath(__file__))


def fingerprints(path):
    rows = {}
    for line in open(path):
        digest, _, name = line.rstrip('\n').partition('\t')
        rows[name] = digest
    return rows


def main():
    base, after = fingerprints(sys.argv[1]), fingerprints(sys.argv[2])

    mixed = {}
    with open(os.path.join(HERE, '..', 'tablerow-r146', 'census-pdf.tsv')) as handle:
        for row in csv.DictReader(handle, delimiter='\t'):
            mixed[row['path']] = int(row['mixed_boundaries'])

    manifest = [line.split('\t')[2]
                for line in open(os.path.join(CORPUS, 'MANIFEST.tsv')).read().splitlines()[1:]
                if line.split('\t')[0] == 'words']

    missing = [p for p in manifest if p not in base or p not in after]
    if missing:
        print('REFUSING TO SCORE: %d manifest paths have no row' % len(missing))
        return

    movers = [p for p in manifest if base[p] != after[p]]
    failed = [p for p in manifest if 'FAILED' in (base[p], after[p])]

    with_mixed = [p for p in movers if mixed.get(p, 0) > 0]
    without = [p for p in movers if mixed.get(p, 0) == 0]
    predicted = [p for p in manifest if mixed.get(p, 0) > 0]

    print('documents scored: %d' % len(manifest))
    print('failed on either leg: %d' % len(failed))
    print('byte-identical: %d' % (len(manifest) - len(movers)))
    print('moved: %d' % len(movers))
    print()
    print('the reference draws a mixed-width boundary in: %d' % len(predicted))
    print('   of those, moved: %d' % len(with_mixed))
    print('   of those, did NOT move: %d' % (len(predicted) - len(with_mixed)))
    print('moved WITHOUT one (unexplained): %d' % len(without))
    for p in without:
        print('      %s' % p)
    print()
    for p in sorted(with_mixed, key=lambda q: -mixed[q])[:20]:
        print('   %5d mixed boundaries  %s' % (mixed[p], p))


if __name__ == '__main__':
    main()
