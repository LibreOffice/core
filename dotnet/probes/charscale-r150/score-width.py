#!/usr/bin/env python3
"""How close each leg's drawn span WIDTHS are to 26.2.4.2's.

`score.py` beside this scores the left EDGE, which is what `probes/odscentre-r150` needed because a
print-centring defect displaces a block. It is the wrong instrument here and says so loudly: it
returns exactly 0.100 pt for all thirteen documents on both legs, which is the two PDF writers' own
constant text origin and nothing else. A character width does not move where a span starts; it
changes how WIDE the span is.

So: pair spans by (page, text) in draw order and compare `x1 - x0`. Reported as the mean absolute
width error over the paired spans, which is in points per span and therefore comparable between a
72-span document and a 19607-span one.
"""
import difflib
import pathlib
import statistics

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
                        out.append((number, text, span['bbox'][2] - span['bbox'][0]))
    return out


def error(reference, ours):
    keys = [(p, t) for p, t, _ in reference]
    mine = [(p, t) for p, t, _ in ours]
    gaps = []
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(a=keys, b=mine, autojunk=False).get_opcodes():
        if tag != 'equal':
            continue
        for i, j in zip(range(i1, i2), range(j1, j2)):
            gaps.append(abs(ours[j][2] - reference[i][2]))
    return (statistics.mean(gaps), len(gaps)) if gaps else (None, 0)


def main():
    rows = []
    for line in pathlib.Path('movers-paths.tsv').read_text().splitlines():
        key, path = line.split('\t')
        ref = list((ROOT / 'ref' / key).glob('*.pdf'))
        if not ref:
            continue
        r = spans(ref[0])
        before, nb = error(r, spans(next((ROOT / 'base' / key).glob('*.pdf'))))
        after, na = error(r, spans(next((ROOT / 'head' / key).glob('*.pdf'))))
        if before is None or after is None:
            continue
        rows.append((before, after, nb, na, pathlib.PurePath(path).name))

    better = sum(1 for b, a, _, _, _ in rows if a < b * 0.99)
    worse = sum(1 for b, a, _, _, _ in rows if a > b * 1.01)
    print('scored %d movers on span WIDTH' % len(rows))
    print('  mean |width error| per span, median over the documents: %7.4f pt  ->  %7.4f pt'
          % (statistics.median([b for b, _, _, _, _ in rows]),
             statistics.median([a for _, a, _, _, _ in rows])))
    print('  closer %d   further %d   level %d' % (better, worse, len(rows) - better - worse))
    print()
    print('  %9s %9s %8s %8s  %s' % ('before', 'after', 'paired', 'paired', 'document'))
    for b, a, nb, na, name in sorted(rows, key=lambda r: r[0] - r[1], reverse=True):
        print('  %9.4f %9.4f %8d %8d  %s' % (b, a, nb, na, name[:58]))


if __name__ == '__main__':
    main()
