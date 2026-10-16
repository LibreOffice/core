#!/usr/bin/env python3
"""How many of a rendering's text lines sit where the reference's do.

A tab is a position defect that adds no glyphs and no pages, so the gate cannot see it and an
ink percentage only says that something moved.  This counts lines that agree on page, start
position (to a tenth of a point) and text, which is what "we now put it where the reference
does" means, and reports the score for two of our renderings against one reference.
"""
import os, sys, pymupdf

def lines(path):
    out = []
    with pymupdf.open(path) as doc:
        for pno, page in enumerate(doc):
            for b in page.get_text('dict')['blocks']:
                for l in b.get('lines', []):
                    t = ''.join(s['text'] for s in l['spans']).strip()
                    if not t:
                        continue
                    s0 = l['spans'][0]['origin']
                    out.append((pno, round(s0[0], 1), round(s0[1], 1), t))
    return out

def score(ours, ref):
    from collections import Counter
    a, b = Counter(ours), Counter(ref)
    same = sum((a & b).values())
    return same, len(ours), len(ref)

before_dir, after_dir, ref_dir, listing = sys.argv[1:5]
tot = [0, 0, 0, 0]
print(f"{'agree/ours/ref before':>34}   {'after':>22}   document")
for name in [l.strip() for l in open(listing) if l.strip().endswith('.pdf')]:
    r = os.path.join(ref_dir, name)
    if not os.path.exists(r):
        continue
    ref = lines(r)
    sb = score(lines(os.path.join(before_dir, name)), ref)
    sa = score(lines(os.path.join(after_dir, name)), ref)
    tot[0] += sb[0]; tot[1] += sa[0]; tot[2] += sb[1]; tot[3] += len(ref)
    flag = '' if sa[0] == sb[0] else ('  BETTER' if sa[0] > sb[0] else '  WORSE')
    print(f"{sb[0]:>10}/{sb[1]}/{sb[2]}   {sa[0]:>10}/{sa[1]}   {name[:52]}{flag}")
print(f"TOTAL agreeing lines before {tot[0]}  after {tot[1]}  of {tot[3]} reference lines")
