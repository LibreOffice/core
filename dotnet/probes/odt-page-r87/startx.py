#!/usr/bin/env python3
"""How far each line's start sits from where the reference puts the same line.

A tab defect moves a line's start and nothing else, so the quantity to measure is the start x
of matched lines.  Lines are matched by their text through difflib rather than by index, so a
document that paginates differently still contributes its matched lines.  Reports the mean
absolute offset and the share within a tenth of a point, for two of our renderings against one
reference.
"""
import difflib, os, sys, pymupdf

def lines(path):
    out = []
    with pymupdf.open(path) as doc:
        for page in doc:
            for b in page.get_text('dict')['blocks']:
                for l in b.get('lines', []):
                    t = ''.join(s['text'] for s in l['spans']).strip()
                    if t:
                        out.append((t, min(s['origin'][0] for s in l['spans'])))
    return out

def compare(ours, ref):
    ta = [t for t, _ in ours]; tb = [t for t, _ in ref]
    sm = difflib.SequenceMatcher(None, ta, tb, autojunk=False)
    n = 0; tot = 0.0; near = 0
    for i, j, size in sm.get_matching_blocks():
        for k in range(size):
            d = abs(ours[i + k][1] - ref[j + k][1])
            n += 1; tot += d
            if d <= 0.1:
                near += 1
    return n, (tot / n if n else 0.0), near

before_dir, after_dir, ref_dir, listing = sys.argv[1:5]
tb = ta = 0.0; nb = na = 0; kb = ka = 0
print(f"{'matched':>8} {'meanΔx before':>14} {'after':>9} {'within 0.1pt before/after':>26}   document")
for name in [l.strip() for l in open(listing) if l.strip().endswith('.pdf')]:
    r = os.path.join(ref_dir, name)
    if not os.path.exists(r):
        continue
    ref = lines(r)
    n1, m1, c1 = compare(lines(os.path.join(before_dir, name)), ref)
    n2, m2, c2 = compare(lines(os.path.join(after_dir, name)), ref)
    nb += n1; na += n2; tb += m1 * n1; ta += m2 * n2; kb += c1; ka += c2
    flag = ''
    if abs(m1 - m2) > 0.005:
        flag = '  BETTER' if m2 < m1 else '  WORSE'
    print(f"{n1:>8} {m1:>14.3f} {m2:>9.3f} {c1:>12}/{c2:<12}   {name[:46]}{flag}")
print(f"TOTAL matched {nb}/{na}  mean |dx| before {tb/max(nb,1):.3f} pt  after {ta/max(na,1):.3f} pt"
      f"  within 0.1 pt {kb} -> {ka}")
