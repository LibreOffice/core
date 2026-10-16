#!/usr/bin/env python3
"""Reconciled census of horizontal table boundaries carrying more than one band width.

    mixed-census.py <pairs.tsv> <out-prefix>

`pairs.tsv` is `rel<TAB>pdf` per line.

A **horizontal boundary** is a maximal chain of horizontal band segments on one page that
share one top edge and abut in x (gap or overlap <= 1.0 pt).  26.2.4.2 top-aligns every
horizontal table band (round 146 §3.1), so a boundary carrying two border widths is one shared
top with two thicknesses hanging off it.  The boundary is MIXED when that chain carries more
than one thickness.  The base rate is the chain of two or more segments all of ONE thickness,
which a model that centres one band per boundary draws correctly.

Four variants are reported so the two earlier instruments can be read off this one:

  a_excl   the chain rule above, with segments shorter than MINLEN pt dropped   <- the answer
  a_all    the same with no length floor (line-art hairlines included)
  ml       `tbalign-r147/mixed-lines.py`: exact rounded top, x ignored entirely
  b_thick  `tablerow-r146/census-pdf.py` shape (b) WITH the thickness test it is missing:
           a narrower group 0 < dy <= 6 pt below a wider one, x-contained, carrying a
           thickness the upper group does not

Segment sources are both PDF path operators the reference uses for a rule: a filled/stroked
`re` and a stroked `l`.  A stroke-only reader answers zero on documents that are drawn as
filled rectangles, which is r147's own warning and is honoured here.
"""
import collections
import sys

import pymupdf

MINLEN = 12.0      # pt; a table rule shorter than this is not a column band
MAXTH = 6.0        # pt; a band thicker than this is a fill, not a rule
GAP = 1.0          # pt; segments this close in x are side by side on one boundary


def segs(page):
    """Horizontal band segments as (top, x0, x1, thickness)."""
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're' and k in ('f', 'fs', 's'):
                r = it[1]
                if r.height <= MAXTH and r.width >= 4.0 and r.width > r.height:
                    out.append((r.y0, r.x0, r.x1, r.height))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                w = d.get('width') or 0.0
                if abs(a.y - b.y) <= 0.01 and abs(a.x - b.x) >= 4.0 and w <= MAXTH:
                    out.append(((a.y + b.y) / 2 - w / 2, min(a.x, b.x), max(a.x, b.x), w))
    return out


def chains(H, minlen):
    """Group by exact top edge, then split each top into x-abutting chains."""
    g = collections.defaultdict(set)
    for top, x0, x1, th in H:
        if x1 - x0 < minlen:
            continue
        g[round(top, 2)].add((round(x0, 2), round(x1, 2), round(th, 3)))
    for top, v in g.items():
        v = sorted(v)
        cur = [v[0]]
        for s in v[1:]:
            if s[0] - max(t[1] for t in cur) <= GAP:
                cur.append(s)
            else:
                yield top, cur
                cur = [s]
        yield top, cur


def score_chains(H, minlen):
    mix = same = 0
    for _top, c in chains(H, minlen):
        if len(c) < 2:
            continue
        if len({t[2] for t in c}) > 1:
            mix += 1
        else:
            same += 1
    return mix, same


def score_ml(H):
    """mixed-lines.py semantics: exact rounded top, no x test at all."""
    g = collections.defaultdict(set)
    for top, _x0, _x1, th in H:
        g[round(top, 2)].add(round(th, 3))
    return sum(1 for w in g.values() if len(w) > 1), len(g)


def cover(v):
    tot, cur0, cur1 = 0.0, None, None
    for x0, x1, _ in sorted(v):
        if cur1 is None or x0 > cur1 + 0.5:
            if cur1 is not None:
                tot += cur1 - cur0
            cur0, cur1 = x0, x1
        else:
            cur1 = max(cur1, x1)
    if cur1 is not None:
        tot += cur1 - cur0
    return tot


def score_b_thick(H):
    """census-pdf.py shape (b) with the thickness test it never performs."""
    g = collections.defaultdict(list)
    for top, x0, x1, th in H:
        key = round(top / 0.06)
        s = (round(x0, 2), round(x1, 2), round(th, 3))
        if s not in g[key]:
            g[key].append(s)
    keys = sorted(g)
    n = 0
    for i, k in enumerate(keys):
        cv = cover(g[k])
        up = {t[2] for t in g[k]}
        for k2 in keys[i + 1:]:
            dy = (k2 - k) * 0.06
            if dy <= 0 or dy > 6.0:
                break
            if (cover(g[k2]) < cv - 1.0
                    and all(any(a[0] >= b[0] - 1.0 and a[1] <= b[1] + 1.0 for b in g[k])
                            for a in g[k2])
                    and {t[2] for t in g[k2]} - up):
                n += 1
                break
    return n


def main():
    rows = []
    for line in open(sys.argv[1]):
        rel, pdf = line.rstrip('\n').split('\t')
        try:
            doc = pymupdf.open(pdf)
        except Exception as e:
            rows.append((rel, -1, -1, -1, -1, -1, -1, f'open:{e}'))
            continue
        ax = asame = aall = ml = mltot = bt = 0
        for p in range(doc.page_count):
            H = segs(doc[p])
            m, s = score_chains(H, MINLEN)
            ax += m
            asame += s
            aall += score_chains(H, 0.0)[0]
            a, b = score_ml(H)
            ml += a
            mltot += b
            bt += score_b_thick(H)
        rows.append((rel, ax, asame, aall, ml, mltot, bt, ''))
        doc.close()
        print('.', end='', flush=True)
    print()

    with open(sys.argv[2] + '.tsv', 'w') as f:
        f.write('path\ta_excl\ta_same_base\ta_all\tml\tml_groups\tb_thick\tnote\n')
        for r in rows:
            f.write('\t'.join(str(x) for x in r) + '\n')

    n = len(rows)
    def docs(i): return sum(1 for r in rows if r[i] > 0)
    def tot(i): return sum(r[i] for r in rows if r[i] > 0)
    print(f'documents read                            {n}')
    print(f'  a_excl  >=1 mixed boundary              {docs(1):4d} docs  {tot(1):6d} boundaries')
    print(f'  base    >=1 one-width column boundary   {docs(2):4d} docs  {tot(2):6d} boundaries')
    print(f'  a_all   (no length floor)               {docs(3):4d} docs  {tot(3):6d} boundaries')
    print(f'  ml      (mixed-lines semantics)         {docs(4):4d} docs  {tot(4):6d} boundaries')
    print(f'  ml_groups (its base)                    {docs(5):4d} docs  {tot(5):6d} groups')
    print(f'  b_thick (shape b, thickness enforced)   {docs(6):4d} docs  {tot(6):6d} boundaries')


if __name__ == '__main__':
    main()
