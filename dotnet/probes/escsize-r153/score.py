#!/usr/bin/env python3
"""How close each leg's drawn spans are to 26.2.4.2's, in BOTH axes, with the gate's own columns.

An escaped run's size decides its advance, so the first-order effect is horizontal -- a following
word's origin -- and the second-order one is vertical, through a line that no longer wraps and a
table row that is no longer two lines tall. Round 152 recorded the general form of the trap the
other way round: a metric aimed at one axis returns the same number on both legs and reads as "no
effect", so both are reported here.

Spans are paired by (page, text) in draw order with `difflib`, which is the pairing every round in
this family has used; a page count difference therefore drops the tail rather than misattributing
it, and the page and alphanumeric columns are reported beside the distances for that reason.
"""
import difflib
import pathlib
import statistics
import sys

import pymupdf

ROOT = pathlib.Path('/home/user/escsize-r153-sweep')


def spans(path):
    out, alnum = [], 0
    document = pymupdf.open(path)
    for number, page in enumerate(document):
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                for span in line['spans']:
                    text = span['text'].strip()
                    alnum += sum(1 for c in span['text'] if c.isalnum())
                    if text:
                        out.append((number, text, span['origin'][0], span['origin'][1]))
    return out, document.page_count, alnum


def error(reference, ours):
    key = lambda rows: [(p, t) for p, t, _, _ in rows]
    dx, dy = [], []
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(
            a=key(reference), b=key(ours), autojunk=False).get_opcodes():
        if tag != 'equal':
            continue
        for i, j in zip(range(i1, i2), range(j1, j2)):
            dx.append(abs(ours[j][2] - reference[i][2]))
            dy.append(abs(ours[j][3] - reference[i][3]))
    if not dx:
        return None
    return statistics.mean(dx), statistics.mean(dy), len(dx)


def main():
    rows = []
    for line in pathlib.Path('movers-paths.tsv').read_text().splitlines():
        key, path = line.split('\t')
        found = list((ROOT / 'ref' / key).glob('*.pdf'))
        if not found:
            continue
        try:
            reference, rp, ra = spans(found[0])
            base, bp, ba = spans(next((ROOT / 'base' / key).glob('*.pdf')))
            head, hp, ha = spans(next((ROOT / 'head' / key).glob('*.pdf')))
        except StopIteration:
            continue
        b, a = error(reference, base), error(reference, head)
        if b is None or a is None:
            continue
        rows.append((b, a, (rp, bp, hp), (ra, ba, ha), pathlib.PurePath(path).name))

    print('scored %d movers' % len(rows))
    for axis, idx in (('dx', 0), ('dy', 1)):
        closer = sum(1 for b, a, *_ in rows if a[idx] < b[idx] - 0.01)
        further = sum(1 for b, a, *_ in rows if a[idx] > b[idx] + 0.01)
        print('  mean |%s| per span, median over the documents: %8.4f pt  ->  %8.4f pt   '
              'closer %d  further %d  level %d'
              % (axis, statistics.median([b[idx] for b, *_ in rows]),
                 statistics.median([a[idx] for _, a, *_ in rows]),
                 closer, further, len(rows) - closer - further))
    pb = sum(1 for _, _, (rp, bp, hp), _, _ in rows if abs(hp - rp) < abs(bp - rp))
    pw = sum(1 for _, _, (rp, bp, hp), _, _ in rows if abs(hp - rp) > abs(bp - rp))
    gb = sum(1 for _, _, _, (ra, ba, ha), _ in rows if abs(ha - ra) < abs(ba - ra))
    gw = sum(1 for _, _, _, (ra, ba, ha), _ in rows if abs(ha - ra) > abs(ba - ra))
    print('  page count:        closer on %d, further on %d' % (pb, pw))
    print('  alphanumerics:     closer on %d, further on %d' % (gb, gw))
    print()
    print('  %8s %8s %8s %8s  %5s %5s %5s  %7s %7s %7s  %s'
          % ('dx bef', 'dx aft', 'dy bef', 'dy aft', 'refp', 'basep', 'headp',
             'ref an', 'base an', 'head an', 'document'))
    for b, a, (rp, bp, hp), (ra, ba, ha), name in sorted(rows, key=lambda r: r[1][0] - r[0][0]):
        print('  %8.4f %8.4f %8.4f %8.4f  %5d %5d %5d  %7d %7d %7d  %s'
              % (b[0], a[0], b[1], a[1], rp, bp, hp, ra, ba, ha, name[:46]))


if __name__ == '__main__':
    main()
