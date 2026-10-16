#!/usr/bin/env python3
"""Read the fit probe: for each ARM, how many lines did the cell take?

Arms are found by their own label paragraph, so nothing is attributed by a y-range guess: an arm
owns every text line between its label and the next label.
"""
import sys, re, pymupdf

doc = pymupdf.open(sys.argv[1])
items = []           # (page, y, x0, x1, text)
for pno in range(doc.page_count):
    for b in doc[pno].get_text('dict')['blocks']:
        for l in b.get('lines', []):
            txt = ''.join(s['text'] for s in l['spans'])
            x0 = min(s['bbox'][0] for s in l['spans'])
            x1 = max(s['bbox'][2] for s in l['spans'])
            items.append((pno, round(l['bbox'][1], 3), x0, x1, txt))
items.sort()

arms = []
for i, (p, y, x0, x1, t) in enumerate(items):
    m = re.match(r'ARM (\S+)', t.strip())
    if m:
        arms.append((m.group(1), i))
print('arm\tlines\twidest_pt\tcontent')
for k, (name, i) in enumerate(arms):
    j = arms[k + 1][1] if k + 1 < len(arms) else len(items)
    body = [it for it in items[i + 1:j] if it[4].strip()]
    widest = max((it[3] - it[2] for it in body), default=0.0)
    print(f'{name}\t{len(body)}\t{widest:.3f}\t{" | ".join(it[4].strip() for it in body)}')
