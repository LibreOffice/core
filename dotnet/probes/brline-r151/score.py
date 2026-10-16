#!/usr/bin/env python3
"""How close each leg's drawn BASELINES are to 26.2.4.2's.

Vertical, not horizontal: this change alters how tall a line holding a manual break is, which moves
every baseline below it. Round 150 learnt the general form of this the hard way -- a metric aimed at
the wrong axis returns the same number for every document on both legs and reads as "no effect".

Spans are paired by (page, text) in draw order and scored on |y|. Reported as the mean over paired
spans, and separately as the page count, because a line-height change is the kind that moves one.
"""
import difflib
import pathlib
import statistics

import pymupdf

ROOT = pathlib.Path('/home/user/brline-r151-sweep')


def spans(path):
    out = []
    document = pymupdf.open(path)
    for number, page in enumerate(document):
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                for span in line['spans']:
                    text = span['text'].strip()
                    if text:
                        out.append((number, text, span['origin'][1]))
    return out, document.page_count


def error(reference, ours):
    key = lambda rows: [(p, t) for p, t, _ in rows]
    gaps = []
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(
            a=key(reference), b=key(ours), autojunk=False).get_opcodes():
        if tag != 'equal':
            continue
        for i, j in zip(range(i1, i2), range(j1, j2)):
            gaps.append(abs(ours[j][2] - reference[i][2]))
    return (statistics.mean(gaps), len(gaps)) if gaps else (None, 0)


def main():
    rows = []
    pages_better = pages_worse = 0
    for line in pathlib.Path('movers-paths.tsv').read_text().splitlines():
        key, path = line.split('\t')
        found = list((ROOT / 'ref' / key).glob('*.pdf'))
        if not found:
            continue
        reference, rp = spans(found[0])
        base, bp = spans(next((ROOT / 'base' / key).glob('*.pdf')))
        head, hp = spans(next((ROOT / 'head' / key).glob('*.pdf')))
        before, nb = error(reference, base)
        after, na = error(reference, head)
        if before is None or after is None:
            continue
        if abs(hp - rp) < abs(bp - rp):
            pages_better += 1
        elif abs(hp - rp) > abs(bp - rp):
            pages_worse += 1
        rows.append((before, after, nb, na, rp, bp, hp, pathlib.PurePath(path).name))

    closer = sum(1 for b, a, *_ in rows if a < b - 0.01)
    further = sum(1 for b, a, *_ in rows if a > b + 0.01)
    print('scored %d movers on BASELINE' % len(rows))
    print('  mean |dy| per span, median over the documents: %8.4f pt  ->  %8.4f pt'
          % (statistics.median([b for b, *_ in rows]),
             statistics.median([a for _, a, *_ in rows])))
    print('  closer %d   further %d   level %d' % (closer, further, len(rows) - closer - further))
    print('  page count: closer to the reference on %d, further on %d' % (pages_better, pages_worse))
    print()
    print('  %9s %9s  %5s %5s %5s  %s' % ('before', 'after', 'ref', 'base', 'head', 'document'))
    for b, a, nb, na, rp, bp, hp, name in sorted(rows, key=lambda r: r[1] - r[0]):
        print('  %9.4f %9.4f  %5d %5d %5d  %s' % (b, a, rp, bp, hp, name[:52]))


if __name__ == '__main__':
    main()
