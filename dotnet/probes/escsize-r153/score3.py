#!/usr/bin/env python3
"""The sharp corpus instrument: compare the SIZE each leg draws an escaped run at with 26.2.4.2's.

26.2.4.2's PDF writer prints every `Tf` at a whole tenth of a point (§1), so the reference's own
drawn size cannot be read exactly -- but it can be PREDICTED exactly from ours: if we set the same
size the layout did, then rounding our size to a tenth of a point must reproduce the reference's
`Tf`. The two candidate rules differ by one twip, 0.05 pt, which straddles a tenth boundary in
exactly the discriminating cases (92 tw -> 4.6, 93 tw -> 4.65 -> 4.7), so this separates them.

Restricted to the spans whose size differs between the two legs, which are the escaped runs.
"""
import difflib
import math
import pathlib

import pymupdf

ROOT = pathlib.Path('/home/user/escsize-r153-sweep')


def spans(path):
    out = []
    for number, page in enumerate(pymupdf.open(path)):
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                for span in line['spans']:
                    if span['text'].strip():
                        out.append((number, span['text'].strip(), span['size']))
    return out


def pair(a, b):
    key = lambda rows: [(p, t) for p, t, _ in rows]
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(
            a=key(a), b=key(b), autojunk=False).get_opcodes():
        if tag == 'equal':
            yield from zip(range(i1, i2), range(j1, j2))


def tenth(pt):
    """Round a size to a tenth of a point THROUGH ITS TWIP COUNT.

    Doing it in floating point instead reads 6.35 pt back as 6.34999... and rounds it to 6.3,
    which is not what the writer does and inverts the verdict on every eleven point superscript in
    the corpus: the first cut of this script scored `150-5370-10H.docx` 0 of 352 for that reason.
    """
    twips = int(math.floor(pt * 20 + 0.5))
    return (twips // 2 + twips % 2) / 10.0


def main():
    rows = []
    tb = th = total = 0
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
        rb, rh = dict(pair(reference, base)), dict(pair(reference, head))
        n = ok_b = ok_h = 0
        for i in set(rb) & set(rh):
            b, h = base[rb[i]][2], head[rh[i]][2]
            if abs(b - h) < 1e-6:
                continue                       # not an escaped run: the legs agree
            n += 1
            ok_b += abs(tenth(b) - reference[i][2]) < 0.005
            ok_h += abs(tenth(h) - reference[i][2]) < 0.005
        if n:
            rows.append((n, ok_b, ok_h, pathlib.PurePath(path).name))
            total += n
            tb += ok_b
            th += ok_h

    print('%d documents hold a span whose drawn size moved; %d such spans' % (len(rows), total))
    print('  matching 26.2.4.2\'s own Tf, rounded to a tenth:  base %d/%d (%.1f%%)   head %d/%d (%.1f%%)'
          % (tb, total, 100.0 * tb / total, th, total, 100.0 * th / total))
    better = sum(1 for n, b, h, _ in rows if h > b)
    worse = sum(1 for n, b, h, _ in rows if h < b)
    print('  documents better %d, worse %d, level %d' % (better, worse, len(rows) - better - worse))
    print()
    print('  %7s %7s %7s  %s' % ('spans', 'base ok', 'head ok', 'document'))
    for n, b, h, name in sorted(rows, key=lambda r: r[1] - r[2]):
        print('  %7d %7d %7d  %s' % (n, b, h, name[:60]))


if __name__ == '__main__':
    main()
