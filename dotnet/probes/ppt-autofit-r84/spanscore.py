#!/usr/bin/env python3
"""Score two renderings of a document against the reference, span for span.

The gate cannot see this defect -- a hyperlink drawn black has exactly the alphanumeric
characters it has drawn blue -- so the measurement is the drawn spans themselves.

Two scores per document, because this round changes both where a span starts and what
colour it is:

  placed    how many of the reference's spans we draw with the same text on the same page
            at the same origin, within TOL points
  coloured  the same, and in the same colour

`pptx-field-r82/score.py` is the ancestor; the colour column is new here because a `.ppt`
hyperlink's most visible consequence is the scheme colour rather than the break.
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
                    out.append((pno, round(s['origin'][1], 3), round(s['origin'][0], 3),
                                t, s['color']))
    doc.close()
    return sorted(out)

def agree(ref, ours, colour):
    if ref is None or ours is None: return 0, len(ref or [])
    pool = {}
    for p, y, x, t, c in ours:
        pool.setdefault((p, t, c) if colour else (p, t), []).append((y, x))
    hit = 0
    for p, y, x, t, c in ref:
        cands = pool.get((p, t, c) if colour else (p, t))
        if not cands: continue
        for i, (yy, xx) in enumerate(cands):
            if abs(yy - y) <= TOL and abs(xx - x) <= TOL:
                hit += 1; cands.pop(i); break
    return hit, len(ref)

if __name__ == '__main__':
    ref, base, head = sys.argv[1], sys.argv[2], sys.argv[3]
    R, B, H = spans(ref), spans(base), spans(head)
    pb, n = agree(R, B, False)
    ph, _ = agree(R, H, False)
    cb, _ = agree(R, B, True)
    ch, _ = agree(R, H, True)
    print(f'{n}\t{pb}\t{ph}\t{cb}\t{ch}')
