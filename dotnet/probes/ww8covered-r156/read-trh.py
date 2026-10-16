#!/usr/bin/env python3
"""For each arm of fixture 6, the two rows' drawn heights, taken from the three grid lines."""
import sys, re, pymupdf

def rules(page):
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 3.0 and r.width >= 20.0 and k in ('f', 'fs', 's'):
                    out.append(((r.y0 + r.y1) / 2, r.height))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 3.0 and abs(a.x - b.x) >= 20.0:
                    out.append(((a.y + b.y) / 2, d.get('width') or 0.0))
    merged = []
    for y, w in sorted(out):
        if merged and y - merged[-1] <= 0.4: continue
        merged.append(y)
    return merged

doc = pymupdf.open(sys.argv[1])
labels = []
for pno in range(doc.page_count):
    for b in doc[pno].get_text('dict')['blocks']:
        for l in b.get('lines', []):
            t = ''.join(s['text'] for s in l['spans']).strip()
            m = re.match(r'ARM (\S+)$', t)
            if m: labels.append((pno, l['bbox'][3], m.group(1)))
labels.sort()
lines = {p: rules(doc[p]) for p in range(doc.page_count)}
print('arm\tupper_h\tlower_h')
for i, (pno, y, name) in enumerate(labels):
    end = labels[i + 1][1] if i + 1 < len(labels) and labels[i + 1][0] == pno else 1e9
    ys = [v for v in lines[pno] if y <= v <= end]
    if len(ys) >= 3:
        print(f'{name}\t{ys[1]-ys[0]:.2f}\t{ys[2]-ys[1]:.2f}')
    else:
        print(f'{name}\t?\t? ({[round(v,2) for v in ys]})')
