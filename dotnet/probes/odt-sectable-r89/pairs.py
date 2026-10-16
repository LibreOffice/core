#!/usr/bin/env python3
"""Two renderings of one probe side by side: the lines whose text matches a pattern.

Line starts and ends, page by page, from both PDFs -- the measurement that says which column a
stretch was laid out in and how wide the measure it wrapped against was.
"""
import re, sys, pymupdf

def rows(path):
    out = []
    with pymupdf.open(path) as d:
        for pno, page in enumerate(d):
            for b in page.get_text('dict')['blocks']:
                for l in b.get('lines', []):
                    t = ''.join(s['text'] for s in l['spans']).strip()
                    if t:
                        out.append((pno, l['spans'][0]['bbox'][0],
                                    max(s['bbox'][2] for s in l['spans']),
                                    l['spans'][0]['origin'][1], t))
    return out

if __name__ == '__main__':
    pat = re.compile(sys.argv[1])
    for path in sys.argv[2:]:
        r = rows(path)
        print(f'== {path}  {1 + max(p for p, *_ in r) if r else 0} pages')
        for pno, x0, x1, y, t in r:
            if pat.search(t):
                print(f'   p{pno} {x0:7.2f}..{x1:7.2f} y={y:7.2f}  {t[:44]}')
