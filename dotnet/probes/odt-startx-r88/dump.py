#!/usr/bin/env python3
"""Dump every text line of a PDF as (page, x0, y0, text), or match two PDFs' lines."""
import sys, difflib, pymupdf

def lines(path):
    out = []
    with pymupdf.open(path) as doc:
        for pno, page in enumerate(doc):
            for b in page.get_text('dict')['blocks']:
                for l in b.get('lines', []):
                    t = ''.join(s['text'] for s in l['spans']).strip()
                    if t:
                        out.append((pno, min(s['origin'][0] for s in l['spans']),
                                    min(s['origin'][1] for s in l['spans']), t))
    return out

if sys.argv[1] == 'one':
    for p, x, y, t in lines(sys.argv[2]):
        print(f"{p:>3} {x:9.2f} {y:9.2f}  {t[:90]}")
else:
    a, b = lines(sys.argv[1]), lines(sys.argv[2])
    ta = [t for *_, t in a]; tb = [t for *_, t in b]
    sm = difflib.SequenceMatcher(None, ta, tb, autojunk=False)
    for i, j, size in sm.get_matching_blocks():
        for k in range(size):
            p1, x1, y1, t = a[i+k]; p2, x2, y2, _ = b[j+k]
            print(f"{x1-x2:9.2f} | ours p{p1} {x1:8.2f},{y1:8.2f} | ref p{p2} {x2:8.2f},{y2:8.2f} | {t[:70]}")
