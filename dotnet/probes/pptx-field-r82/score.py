#!/usr/bin/env python3
"""Score two renderings of a document against the reference, span for span.

The gate cannot see this defect -- a re-broken URL has the same characters -- so the
measurement is the drawn spans themselves: how many of ours carry exactly the reference's
text at exactly the reference's baseline.  A span is (page, baseline, x, text); two spans
agree when the text is identical and the origin is within `TOL` points, which is above the
0.028 pt constant the two stacks differ by everywhere and well below a line.
"""
import sys

try:                       # the `fitz` alias prints a deprecation line on stdout
    import pymupdf as fitz
except ImportError:
    import fitz

TOL = 0.30

def spans(path):
    out = []
    try:
        doc = fitz.open(path)
    except Exception:
        return None
    for pno, page in enumerate(doc):
        for b in page.get_text('dict')['blocks']:
            if b['type'] != 0: continue
            for line in b['lines']:
                for s in line['spans']:
                    t = s['text'].rstrip()
                    if not t: continue
                    out.append((pno, round(s['origin'][1], 3), round(s['origin'][0], 3), t))
    doc.close()
    return sorted(out)

def agree(ref, ours):
    """How many of the reference's spans have a partner in ours."""
    if ref is None or ours is None: return 0, 0
    pool = {}
    for p, y, x, t in ours:
        pool.setdefault((p, t), []).append((y, x))
    hit = 0
    for p, y, x, t in ref:
        cands = pool.get((p, t))
        if not cands: continue
        for i, (yy, xx) in enumerate(cands):
            if abs(yy - y) <= TOL and abs(xx - x) <= TOL:
                hit += 1
                cands.pop(i)
                break
    return hit, len(ref)

if __name__ == '__main__':
    ref, base, head = sys.argv[1], sys.argv[2], sys.argv[3]
    R = spans(ref)
    hb, n = agree(R, spans(base))
    hh, _ = agree(R, spans(head))
    print(f'{hb}\t{hh}\t{n}')
