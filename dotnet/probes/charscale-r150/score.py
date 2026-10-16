#!/usr/bin/env python3
"""How far each leg's text sits from 26.2.4.2's, per document, on the x axis.

Spans are paired by (page, text) in draw order with `difflib`, and the score is the MEDIAN
absolute x distance over the paired spans -- median rather than mean because one stray pairing
should not decide a document, and banded per page rather than averaged across the sheet because
`probes/invcol-r149` showed this rule displacing each PRINTED PAGE by its own amount: averaging
reads a two-page sheet as one meaningless constant.
"""
import collections
import difflib
import pathlib
import statistics
import sys

import pymupdf

ROOT = pathlib.Path('/home/user/charscale-r150-sweep')


def spans(path):
    out = []
    for number, page in enumerate(pymupdf.open(path)):
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                for span in line['spans']:
                    text = span['text'].strip()
                    if text:
                        out.append((number, text, span['bbox'][0]))
    return out


def distance(reference, ours):
    keys = [(p, t) for p, t, _ in reference]
    mine = [(p, t) for p, t, _ in ours]
    matcher = difflib.SequenceMatcher(a=keys, b=mine, autojunk=False)
    gaps = []
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        if tag != 'equal':
            continue
        for i, j in zip(range(i1, i2), range(j1, j2)):
            gaps.append(abs(ours[j][2] - reference[i][2]))
    return (statistics.median(gaps), len(gaps)) if gaps else (None, 0)


def main():
    rows = []
    for line in pathlib.Path('movers-paths.tsv').read_text().splitlines():
        key, path = line.split('\t')
        ref = list((ROOT / 'ref' / key).glob('*.pdf'))
        if not ref:
            continue
        r = spans(ref[0])
        before, _ = distance(r, spans(next((ROOT / 'base' / key).glob('*.pdf'))))
        after, n = distance(r, spans(next((ROOT / 'head' / key).glob('*.pdf'))))
        if before is None or after is None:
            continue
        rows.append((before, after, n, pathlib.PurePath(path).name))

    better = sum(1 for b, a, _, _ in rows if a < b - 0.05)
    worse = sum(1 for b, a, _, _ in rows if a > b + 0.05)
    exact = sum(1 for _, a, _, _ in rows if a <= 0.1)
    was_exact = sum(1 for b, _, _, _ in rows if b <= 0.1)

    print('scored %d of the 81 movers' % len(rows))
    print('  median |dx| over all of them : %7.3f pt  ->  %7.3f pt'
          % (statistics.median([b for b, _, _, _ in rows]),
             statistics.median([a for _, a, _, _ in rows])))
    print('  within 0.1 pt of 26.2.4.2    : %d  ->  %d' % (was_exact, exact))
    print('  closer %d   further %d   level %d' % (better, worse, len(rows) - better - worse))
    print()
    for b, a, n, name in sorted(rows, key=lambda r: r[1] - r[0])[:6]:
        print('  %8.3f -> %8.3f  (%3d spans)  %s' % (b, a, n, name[:64]))
    print('  ...')
    for b, a, n, name in sorted(rows, key=lambda r: r[1] - r[0])[-4:]:
        print('  %8.3f -> %8.3f  (%3d spans)  %s' % (b, a, n, name[:64]))


if __name__ == '__main__':
    main()
