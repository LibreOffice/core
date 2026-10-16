#!/usr/bin/env python3
"""How far each line's start sits from where the reference puts the same line.

`probes/odt-page-r87/startx.py` with two corrections, both of which cost that round real
numbers:

* **A line is a baseline, not a text object.** A justified line is written as one text object
  per stretch in this tree's PDF and as one object per line in 26.2.4.2's, so PyMuPDF's
  `get_text('dict')` reports 635 "lines" against 479 for the same page and the matcher then
  pairs a fragment against a whole line.  Spans are merged on (page, baseline) here, and the
  line's start is the leftmost of them.
* **Identical repeated text is matched in draw order, which is not position order.** A
  template drawing the same label in nine places contributes nine matched pairs in whatever
  order each side emitted them, and `011_Project_Timeline_Template_Beautiful_Theme`'s 341.96 pt
  mean is entirely that — its nine blocks sit at the *same nine* x on both sides, to 0.10 pt.
  Reported beside the mean as the share of matched lines whose text is unique on both sides,
  which is the column to read when a document is a template.
"""
import difflib, os, sys, collections, pymupdf

def lines(path):
    rows = []
    with pymupdf.open(path) as doc:
        for pno, page in enumerate(doc):
            byline = collections.defaultdict(list)
            for b in page.get_text('dict')['blocks']:
                for l in b.get('lines', []):
                    for s in l['spans']:
                        if s['text'].strip():
                            byline[(pno, round(s['origin'][1], 1))].append(s)
            for key in sorted(byline):
                spans = sorted(byline[key], key=lambda s: s['origin'][0])
                text = ''.join(s['text'] for s in spans).strip()
                if text:
                    rows.append((text, spans[0]['origin'][0]))
    return rows

def compare(ours, ref):
    ta = [t for t, _ in ours]; tb = [t for t, _ in ref]
    once = {t for t, n in collections.Counter(ta).items() if n == 1}
    once &= {t for t, n in collections.Counter(tb).items() if n == 1}
    sm = difflib.SequenceMatcher(None, ta, tb, autojunk=False)
    n = uniq = 0; tot = utot = 0.0; near = 0
    for i, j, size in sm.get_matching_blocks():
        for k in range(size):
            d = abs(ours[i + k][1] - ref[j + k][1])
            n += 1; tot += d
            if d <= 0.1: near += 1
            if ours[i + k][0] in once:
                uniq += 1; utot += d
    return n, (tot / n if n else 0.0), near, uniq, (utot / uniq if uniq else 0.0)

def main():
    before_dir, after_dir, ref_dir, listing = sys.argv[1:5]
    tb = ta = 0.0; nb = na = 0; kb = ka = 0
    print(f"{'matched b/a':>13} {'mean|dx| before':>15} {'after':>9} {'within .1pt':>12} "
          f"{'unique':>7} {'mean|dx| unique':>15}   document")
    for name in [l.strip() for l in open(listing) if l.strip().endswith('.pdf')]:
        r = os.path.join(ref_dir, name)
        if not os.path.exists(r):
            continue
        ref = lines(r)
        n1, m1, c1, _, _ = compare(lines(os.path.join(before_dir, name)), ref)
        n2, m2, c2, u2, um2 = compare(lines(os.path.join(after_dir, name)), ref)
        nb += n1; na += n2; tb += m1 * n1; ta += m2 * n2; kb += c1; ka += c2
        flag = ''
        if abs(m1 - m2) > 0.005:
            flag = '  BETTER' if m2 < m1 else '  WORSE'
        print(f"{n1:>6}/{n2:<6} {m1:>15.3f} {m2:>9.3f} {c1:>5}/{c2:<6} {u2:>7} {um2:>15.3f}   "
              f"{name[:46]}{flag}")
    print(f"TOTAL matched {nb}/{na}  mean |dx| before {tb/max(nb,1):.3f} pt "
          f"after {ta/max(na,1):.3f} pt  within 0.1 pt {kb} -> {ka}")

main()
