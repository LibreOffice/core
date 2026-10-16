#!/usr/bin/env python3
"""Line-by-line comparison of two PDFs' drawn spans: baseline, x, colour and text.

The gate cannot see any of this -- a re-broken URL has the same characters -- so it is
the baselines and the break positions themselves that are the measurement.
"""
import sys

try:                       # the `fitz` alias prints a deprecation line on stdout
    import pymupdf as fitz
except ImportError:
    import fitz

def spans(path):
    out = []
    doc = fitz.open(path)
    for page in doc:
        for b in page.get_text('dict')['blocks']:
            if b['type'] != 0: continue
            for line in b['lines']:
                for s in line['spans']:
                    # rstrip: a line's trailing blank is inside the reference's span and
                    # outside ours, and it is not drawn ink either way.
                    out.append((round(s['origin'][1], 3), round(s['origin'][0], 3),
                                s['color'], s['text'].rstrip()))
    doc.close()
    return sorted(out)

a, b = sys.argv[1], sys.argv[2]
tol = float(sys.argv[3]) if len(sys.argv) > 3 else 0.05
A, B = spans(a), spans(b)
ok = len(A) == len(B)
worst = 0.0
for x, y in zip(A, B):
    if x[3] != y[3] or x[2] != y[2]: ok = False
    worst = max(worst, abs(x[0]-y[0]), abs(x[1]-y[1]))
if worst > tol: ok = False
print(f'{"MATCH" if ok else "DIFFER"}\tspans {len(A)}/{len(B)}\tworst {worst:.3f}\t{a} {b}')
if not ok:
    for i in range(max(len(A), len(B))):
        x = A[i] if i < len(A) else None
        y = B[i] if i < len(B) else None
        if x != y:
            print(f'  ref  {x}')
            print(f'  ours {y}')
