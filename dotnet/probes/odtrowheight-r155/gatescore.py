#!/usr/bin/env python3
"""Score the movers on the gate's own two columns as well as on distance.

A row-height change moves pagination, so this column's headline is pages and alphanumeric
characters rather than a sub-point distance: `batch-check.sh` fails a document when the character
difference exceeds BOTH 2 % and a floor of 15, and it requires the page counts to be equal.
"""
import difflib
import pathlib
import statistics

import pymupdf

ROOT = pathlib.Path('/home/user/odtrowheight-r155-sweep')


def read(path):
    spans, alnum = [], 0
    document = pymupdf.open(path)
    for number, page in enumerate(document):
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                for span in line['spans']:
                    alnum += sum(1 for c in span['text'] if c.isalnum())
                    if span['text'].strip():
                        spans.append((number, span['text'].strip(),
                                      span['origin'][0], span['origin'][1]))
    return spans, document.page_count, alnum


def distance(reference, ours):
    key = lambda rows: [(p, t) for p, t, _, _ in rows]
    dy = []
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(
            a=key(reference), b=key(ours), autojunk=False).get_opcodes():
        if tag != 'equal':
            continue
        for i, j in zip(range(i1, i2), range(j1, j2)):
            dy.append(abs(ours[j][3] - reference[i][3]))
    return statistics.mean(dy) if dy else None


def verdict(pages, alnum, refpages, refalnum):
    if pages != refpages:
        return 'pages'
    d = abs(alnum - refalnum)
    return 'match' if not (d > refalnum * 0.02 and d > 15) else 'glyphs'


rows = []
for line in pathlib.Path('movers-paths.tsv').read_text().splitlines():
    key, path = line.split('\t')
    found = list((ROOT / 'ref' / key).glob('*.pdf'))
    if not found:
        continue
    try:
        reference, rp, ra = read(found[0])
        base, bp, ba = read(next((ROOT / 'base' / key).glob('*.pdf')))
        head, hp, ha = read(next((ROOT / 'head' / key).glob('*.pdf')))
    except StopIteration:
        continue
    rows.append((verdict(bp, ba, rp, ra), verdict(hp, ha, rp, ra),
                 distance(reference, base), distance(reference, head),
                 rp, bp, hp, ra, ba, ha, pathlib.PurePath(path).name))

print('scored %d movers of 117' % len(rows))
print('  gate verdict: %d match before, %d after' % (sum(1 for r in rows if r[0] == 'match'),
                                                     sum(1 for r in rows if r[1] == 'match')))
gained = [r for r in rows if r[0] != 'match' and r[1] == 'match']
lost = [r for r in rows if r[0] == 'match' and r[1] != 'match']
print('  gained %d, lost %d' % (len(gained), len(lost)))
pb = sum(1 for r in rows if abs(r[6] - r[4]) < abs(r[5] - r[4]))
pw = sum(1 for r in rows if abs(r[6] - r[4]) > abs(r[5] - r[4]))
print('  page count closer on %d, further on %d' % (pb, pw))
scored = [r for r in rows if r[2] is not None and r[3] is not None]
print('  mean |dy| per span, median over the documents: %.4f -> %.4f pt   closer %d  further %d'
      % (statistics.median([r[2] for r in scored]), statistics.median([r[3] for r in scored]),
         sum(1 for r in scored if r[3] < r[2] - 0.01),
         sum(1 for r in scored if r[3] > r[2] + 0.01)))
print()
print('  %-7s %-7s %9s %9s %5s %5s %5s  %s'
      % ('before', 'after', 'dy bef', 'dy aft', 'refp', 'basep', 'headp', 'document'))
for r in sorted(rows, key=lambda r: (r[1] == 'match') - (r[0] == 'match')):
    print('  %-7s %-7s %9s %9s %5d %5d %5d  %s'
          % (r[0], r[1],
             'n/a' if r[2] is None else '%.4f' % r[2], 'n/a' if r[3] is None else '%.4f' % r[3],
             r[4], r[5], r[6], r[10][:52]))
