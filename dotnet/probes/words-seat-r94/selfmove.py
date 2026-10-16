#!/usr/bin/env python3
"""What one change did to a rendering, measured against our own other leg rather than a reference.

Needed because the three `150_5300_13` revisions sit a page later than 26.2.4.2 does, so a
page-for-page comparison with the reference compares different content and cannot say whether a
change moved anything on them.  Spans are paired by (page, baseline, text) between the two legs,
which is exact when only positions moved, and the report is how many moved and by how much.

    selfmove.py <before-dir> <after-dir> <list-of-pdf-names>
"""
import collections, os, sys, pymupdf

def spans(path):
    out = collections.defaultdict(list)
    with pymupdf.open(path) as doc:
        for pno, page in enumerate(doc):
            for b in page.get_text('dict')['blocks']:
                for l in b.get('lines', []):
                    for s in l['spans']:
                        if s['text'].strip():
                            out[(pno, round(s['origin'][1], 1), s['text'])].append(s['origin'][0])
    return out

def main():
    a, b, listing = sys.argv[1:4]
    print(f"{'spans':>7} {'moved':>7} {'mean dx of movers':>18} {'min':>8} {'max':>8}   document")
    for nm in [l.strip() for l in open(listing) if l.strip().endswith('.pdf')]:
        da, db = spans(os.path.join(a, nm)), spans(os.path.join(b, nm))
        ds = []
        for k, v in da.items():
            if k in db and len(v) == len(db[k]):
                ds += [q - p for p, q in zip(sorted(v), sorted(db[k]))]
        mv = [d for d in ds if abs(d) > 0.01]
        if mv:
            print(f"{len(ds):7d} {len(mv):7d} {sum(mv) / len(mv):18.2f} "
                  f"{min(mv):8.2f} {max(mv):8.2f}   {nm}")
        else:
            print(f"{len(ds):7d} {0:7d} {'-':>18} {'-':>8} {'-':>8}   {nm}")

main()
