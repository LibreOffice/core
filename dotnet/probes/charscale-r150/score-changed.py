#!/usr/bin/env python3
"""Of the spans this change actually moved, how many moved TOWARDS 26.2.4.2?

`score-width.py` beside this averages over every paired span, and on these documents that is mostly
dilution: a `\\charscalex` run is a handful of spans among hundreds, so a real improvement on them is
invisible in a mean and eight of the thirteen documents come out identical to four decimal places.

This restricts the comparison to the spans whose drawn width DIFFERS between the two legs -- the
ones the change reached -- and asks, of those, how many are closer to the reference's width after
than before. A metric that cannot see the change is not evidence that the change did nothing.
"""
import difflib
import pathlib

import pymupdf

ROOT = pathlib.Path('/home/user/charscale-r150-sweep')
TOL = 0.001


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


def triples(reference, base, head):
    """Spans present in all three, as (reference width, base width, head width)."""
    key = lambda rows: [(p, t) for p, t, _ in rows]
    out = []
    ops = difflib.SequenceMatcher(a=key(base), b=key(head), autojunk=False).get_opcodes()
    pairs = {}
    for tag, i1, i2, j1, j2 in ops:
        if tag != 'equal':
            continue
        for i, j in zip(range(i1, i2), range(j1, j2)):
            pairs[key(base)[i]] = (base[i][2], head[j][2])
    for page, text, width in reference:
        if (page, text) in pairs:
            b, h = pairs[(page, text)]
            out.append((width, b, h))
    return out


def main():
    closer = further = same = 0
    docs = []
    for line in pathlib.Path('movers-paths.tsv').read_text().splitlines():
        key, path = line.split('\t')
        ref = list((ROOT / 'ref' / key).glob('*.pdf'))
        if not ref:
            continue
        rows = triples(spans(ref[0]),
                       spans(next((ROOT / 'base' / key).glob('*.pdf'))),
                       spans(next((ROOT / 'head' / key).glob('*.pdf'))))
        moved = [(r, b, h) for r, b, h in rows if abs(h - b) > TOL]
        c = sum(1 for r, b, h in moved if abs(h - r) < abs(b - r) - TOL)
        f = sum(1 for r, b, h in moved if abs(h - r) > abs(b - r) + TOL)
        closer += c
        further += f
        same += len(moved) - c - f
        docs.append((len(moved), c, f, pathlib.PurePath(path).name))

    print('spans whose drawn width this change moved: %d' % (closer + further + same))
    print('  closer to 26.2.4.2 %d   further %d   neither %d' % (closer, further, same))
    print()
    print('  %7s %7s %7s  %s' % ('moved', 'closer', 'further', 'document'))
    for moved, c, f, name in sorted(docs, reverse=True):
        print('  %7d %7d %7d  %s' % (moved, c, f, name[:58]))


if __name__ == '__main__':
    main()
