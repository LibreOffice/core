#!/usr/bin/env python3
"""Read the E-arms: the drawn size of a `w:vertAlign superscript` run against its base run."""
import sys, re, pymupdf
doc = pymupdf.open(sys.argv[1])
spans = []
for pno in range(doc.page_count):
    for b in doc[pno].get_text('dict')['blocks']:
        for l in b.get('lines', []):
            for s in l['spans']:
                spans.append((pno, round(l['bbox'][1], 3), s['text'], round(s['size'], 4)))
spans.sort()
cur = None
print('arm\tbase_hp\tbase_pt\tsuper_pt\tratio\tbase_tw\tsuper_tw')
for pno, y, t, size in spans:
    m = re.match(r'ARM E(\d+)', t.strip())
    if m:
        cur = int(m.group(1)); base = None; continue
    if cur is None: continue
    if t.strip() == 'Mx': base = size
    elif t.strip() == '19' and base:
        print(f'E{cur}\t{cur}\t{base:.4f}\t{size:.4f}\t{size/base:.5f}\t{base*20:.1f}\t{size*20:.2f}')
        cur = None
