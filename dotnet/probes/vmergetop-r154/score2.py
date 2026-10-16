#!/usr/bin/env python3
"""Per-SPAN scoring, because a document mean cannot see this change.

An escaped run's size moves the spans that follow it on its own line and nothing else, so on a
page of a hundred spans a document-level mean of |dx| is diluted by two orders of magnitude and
reads as "level" (round 153's first cut of `score.py` reported 116 of 118 level for exactly that
reason). What answers the question is the spans that actually MOVED between the two legs: of those,
how many did the change put closer to 26.2.4.2 and how many further.
"""
import collections
import difflib
import pathlib
import statistics

import pymupdf

ROOT = pathlib.Path('/home/user/vmergetop-r154-sweep')
EPS = 0.002


def spans(path):
    out = []
    for number, page in enumerate(pymupdf.open(path)):
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                for span in line['spans']:
                    if span['text'].strip():
                        out.append((number, span['text'].strip(),
                                    span['origin'][0], span['origin'][1]))
    return out


def pair(a, b):
    key = lambda rows: [(p, t) for p, t, _, _ in rows]
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(
            a=key(a), b=key(b), autojunk=False).get_opcodes():
        if tag == 'equal':
            yield from zip(range(i1, i2), range(j1, j2))


def main():
    rows = []
    totals = collections.Counter()
    for line in pathlib.Path('movers-paths.tsv').read_text().splitlines():
        key, path = line.split('\t')
        found = list((ROOT / 'ref' / key).glob('*.pdf'))
        if not found:
            continue
        try:
            reference = spans(found[0])
            base = spans(next((ROOT / 'base' / key).glob('*.pdf')))
            head = spans(next((ROOT / 'head' / key).glob('*.pdf')))
        except StopIteration:
            continue
        rb = dict(pair(reference, base))
        rh = dict(pair(reference, head))
        closer = further = moved = 0
        for i in set(rb) & set(rh):
            b, h = base[rb[i]], head[rh[i]]
            if abs(b[2] - h[2]) < EPS and abs(b[3] - h[3]) < EPS:
                continue
            moved += 1
            db = abs(b[2] - reference[i][2]) + abs(b[3] - reference[i][3])
            dh = abs(h[2] - reference[i][2]) + abs(h[3] - reference[i][3])
            if dh < db - EPS:
                closer += 1
            elif dh > db + EPS:
                further += 1
        rows.append((moved, closer, further, pathlib.PurePath(path).name))
        totals['moved'] += moved
        totals['closer'] += closer
        totals['further'] += further

    docs_better = sum(1 for m, c, f, _ in rows if c > f)
    docs_worse = sum(1 for m, c, f, _ in rows if f > c)
    print('%d movers scored; spans that moved between the legs: %d' % (len(rows), totals['moved']))
    print('  of those, CLOSER to 26.2.4.2 %d, FURTHER %d, level %d'
          % (totals['closer'], totals['further'],
             totals['moved'] - totals['closer'] - totals['further']))
    print('  documents net better %d, net worse %d, net level %d'
          % (docs_better, docs_worse, len(rows) - docs_better - docs_worse))
    print()
    print('  %7s %7s %7s  %s' % ('moved', 'closer', 'further', 'document'))
    for m, c, f, name in sorted(rows, key=lambda r: r[2] - r[1]):
        print('  %7d %7d %7d  %s' % (m, c, f, name[:60]))


if __name__ == '__main__':
    main()
