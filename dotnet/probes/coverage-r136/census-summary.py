#!/usr/bin/env python3
"""Cross-tabulate the never-examined set against the corpus's own shape.

    census-summary.py <mentions.tsv> <gate-rows.tsv>

C9: a census with no base rate is not evidence.  Every row here carries the population it is
a share of and the ratio against the whole corpus's share, so "over-represented" is a number.
"""
import collections
import pathlib
import sys

rows = [l.split('\t') for l in
        pathlib.Path(sys.argv[1]).read_text(encoding='utf-8').splitlines()[1:]]
pages = {}
for l in pathlib.Path(sys.argv[2]).read_text(encoding='utf-8').splitlines():
    p = l.split('\t')
    pages[p[0].rsplit('/', 1)[-1].rsplit('.', 1)[0]] = int(p[2].split('/')[1])

base = sum(1 for r in rows if r[0] == '0') / len(rows)


def tab(title, key):
    tot, zero = collections.Counter(), collections.Counter()
    for r in rows:
        k = key(r)
        tot[k] += 1
        zero[k] += r[0] == '0'
    print(f'\n{title}')
    print(f'  {"":24}{"never":>7}{"of":>7}{"share":>8}{"vs corpus":>11}')
    for k in sorted(tot, key=lambda k: -tot[k]):
        s = zero[k] / tot[k]
        print(f'  {str(k) or "(none)":24}{zero[k]:>7}{tot[k]:>7}{s * 100:>7.1f}%{s / base:>10.2f}x')
    print(f'  {"ALL":24}{sum(zero.values()):>7}{sum(tot.values()):>7}{base * 100:>7.1f}%')


def bucket(r):
    n = pages.get(r[1])
    if n is None:
        return 'unknown'
    return '1 page' if n == 1 else '2-3' if n <= 3 else '4-9' if n <= 9 \
        else '10-49' if n <= 49 else '50+'


tab('by extension', lambda r: r[2])
tab('by track', lambda r: r[3])
tab('by batch family', lambda r: r[4].rsplit('-', 1)[0])
tab('by manifest status', lambda r: r[5])
tab('by manifest kind', lambda r: r[6])
tab("by the reference's page count", bucket)
