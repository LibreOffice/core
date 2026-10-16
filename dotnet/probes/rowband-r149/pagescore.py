#!/usr/bin/env python3
"""Page counts for a sweep's movers against the banked 26.2.4.2 renderings.

The gate's first check, and the one O84 was expected to move -- `OPEN-ISSUES.md` calls it "the one
of the three that moves page counts, so it is the one to gate".
"""
import hashlib
import pathlib
import sys

import pymupdf

base = pathlib.Path(sys.argv[1])
head = pathlib.Path(sys.argv[2])
refs = {p.stem: p for p in pathlib.Path(sys.argv[3]).rglob('*.pdf')}
docs = [l.split('\t') for l in pathlib.Path(sys.argv[4]).read_text().splitlines() if l]

better = worse = level = 0
exact_before = exact_after = scored = 0
rows = []
for family, ext, path in docs:
    stem = pathlib.PurePath(path).stem
    if stem not in refs:
        continue
    key = hashlib.md5(('/home/user/sample-files/' + path).encode()).hexdigest()[:12]
    b = list((base / key).glob('*.pdf'))
    h = list((head / key).glob('*.pdf'))
    if not b or not h:
        continue
    r = pymupdf.open(refs[stem]).page_count
    nb = pymupdf.open(b[0]).page_count
    nh = pymupdf.open(h[0]).page_count
    scored += 1
    exact_before += nb == r
    exact_after += nh == r
    if abs(nh - r) < abs(nb - r):
        better += 1
        rows.append(('BETTER', r, nb, nh, path))
    elif abs(nh - r) > abs(nb - r):
        worse += 1
        rows.append(('WORSE ', r, nb, nh, path))
    else:
        level += 1

print('scored %d   page-exact before %d, after %d' % (scored, exact_before, exact_after))
print('closer to the reference %d   further %d   unchanged distance %d' % (better, worse, level))
for verdict, r, nb, nh, path in sorted(rows):
    print('  %s ref %4d  base %4d  head %4d   %s' % (verdict, r, nb, nh, path[-70:]))
