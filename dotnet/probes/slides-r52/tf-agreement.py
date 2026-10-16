#!/usr/bin/env python3
"""How many of the reference's drawn em sizes do we draw?

The gate cannot see a font size and neither can an ink percentage with any directness: a
body drawn two points small moves ink in whichever direction the surrounding page happens
to make it move.  This is the quantity the round's change actually controls.

Per page, take the multiset of /Tf sizes rounded to a tenth of a point, weighted by how many
text-showing operators carry each, and score the overlap against the reference's:

    agreement = sum(min(ours[s], ref[s])) / sum(max(ours[s], ref[s]))

1.0 means every drawn size, in the right proportion.  Summed over pages and averaged.

    tf-agreement.py <ours-dir> <ref-dir> [label]
"""
import collections, glob, os, sys

sys.path.insert(0, "/c/sandbox/workdir/wt-slides-r50/dotnet/research/probes/slides-r15")
from pdfops import dump, objects, pages  # noqa: E402


def sizes(pdf, page):
    try:
        ops = dump(pdf, page)
    except Exception:
        return None
    c = collections.Counter()
    for font, base, sz, tm, td in ops:
        if sz:
            c[round(sz, 1)] += 1
    return c


def page_count(pdf):
    try:
        d = open(pdf, "rb").read()
        return len(pages(d, objects(d)))
    except Exception:
        return 0


ours_dir, ref_dir = sys.argv[1], sys.argv[2]
label = sys.argv[3] if len(sys.argv) > 3 else os.path.basename(ours_dir.rstrip("/"))

rows = []
for o in sorted(glob.glob(os.path.join(ours_dir, "*.pdf"))):
    ident = os.path.basename(o)[:-4]
    r = os.path.join(ref_dir, ident + ".pdf")
    if not os.path.exists(r):
        continue
    n = min(page_count(o), page_count(r))
    if n == 0:
        continue
    tot = hits = 0.0
    exact = 0
    for p in range(1, n + 1):
        a, b = sizes(o, p), sizes(r, p)
        if a is None or b is None:
            continue
        keys = set(a) | set(b)
        lo = sum(min(a[k], b[k]) for k in keys)
        hi = sum(max(a[k], b[k]) for k in keys)
        if hi == 0:
            continue
        tot += 1
        hits += lo / hi
        exact += (a == b)
    if tot:
        rows.append((ident, hits / tot, exact, int(tot)))

print(f"# {label}: {len(rows)} documents")
print("ident\tagreement\texact_pages\tpages")
for ident, agree, exact, n in rows:
    print(f"{ident}\t{agree:.5f}\t{exact}\t{n}")
print(f"\n# mean per-document agreement: {sum(r[1] for r in rows) / len(rows):.5f}")
print(f"# pages whose size multiset is EXACT: {sum(r[2] for r in rows)} of {sum(r[3] for r in rows)}")
