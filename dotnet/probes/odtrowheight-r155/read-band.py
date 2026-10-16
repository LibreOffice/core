#!/usr/bin/env python3
"""Per arm of fixture 8: the two rows' baselines, their gap, and every horizontal rule drawn.

Fixture 8 states `nil` at most boundaries on purpose, so there are not three grid lines to measure
a row height between -- `read-trh.py` reports `?` for seven of its eight arms. What is always there
is the text: the upper row's `u` and the lower row's `l`. Their baseline gap moves with whatever the
lower row pays at the boundary, and the rule list says whether anything was drawn there.
"""
import sys, re, pymupdf

doc = pymupdf.open(sys.argv[1])
marks = []                                  # (page, y, kind, payload)
for pno in range(doc.page_count):
    page = doc[pno]
    for b in page.get_text('dict')['blocks']:
        for l in b.get('lines', []):
            t = ''.join(s['text'] for s in l['spans']).strip()
            y = l['spans'][0]['origin'][1]
            if re.match(r'ARM \S+$', t):
                marks.append((pno, y, 'arm', t.split()[1]))
            elif t in ('u', 'l'):
                marks.append((pno, y, 'text', t))
    for d in page.get_drawings():
        kind = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 3.0 and r.width >= 20.0 and kind in ('f', 'fs', 's'):
                    marks.append((pno, (r.y0 + r.y1) / 2, 'rule', round(r.height, 3)))
            elif it[0] == 'l' and kind in ('s', 'fs'):
                a, b2 = it[1], it[2]
                if abs(a.y - b2.y) <= 3.0 and abs(a.x - b2.x) >= 20.0:
                    marks.append((pno, (a.y + b2.y) / 2, 'rule', round(d.get('width') or 0.0, 3)))
marks.sort()

print('arm\tu_base\tl_base\tgap\trules_between (y@width)')
cur, rows = None, []
def flush(name, items):
    if name is None:
        return
    u = next((y for y, k, p in items if k == 'text' and p == 'u'), None)
    low = next((y for y, k, p in items if k == 'text' and p == 'l'), None)
    if u is None or low is None:
        print(f'{name}\t?\t?\t?\t--')
        return
    between = [f'{y:.2f}@{p}' for y, k, p in items if k == 'rule' and u < y < low]
    print(f'{name}\t{u:.2f}\t{low:.2f}\t{low - u:.2f}\t' + (' '.join(between) or '-none-'))

for pno, y, kind, payload in marks:
    if kind == 'arm':
        flush(cur, rows)
        cur, rows = payload, []
    else:
        rows.append((y, kind, payload))
flush(cur, rows)
