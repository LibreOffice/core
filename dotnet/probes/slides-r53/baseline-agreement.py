#!/usr/bin/env python3
"""How far our text baselines sit from the reference's, per document.

`tf-agreement.py` scores the /Tf sizes we draw; this scores WHERE we draw them, which is the
quantity a line-height change controls and which an ink percentage cannot report directly --
a page that already carries surplus ink can show a HIGHER unsigned ink figure when text moves
onto its correct baseline.

Per page: bucket both sides' text-showing operators by rounded /Tf size, sort each bucket's
distinct baselines, and pair them IN ORDER within a bucket -- not by nearest neighbour, which
is the pairing that manufactured 142 phantom box notes for round 50 and which silently
rewards a shift by matching a line to its neighbour.  Pairs are only formed when the two
buckets hold the same number of baselines, so a page whose line COUNT differs contributes
nothing rather than contributing noise.

    baseline-agreement.py <ours-dir> <ref-dir> [--only substring]
"""
import collections, glob, os, sys
sys.path.insert(0, "/c/sandbox/workdir/wt-slides-r50/dotnet/research/probes/slides-r15")
from pdfops import dump, objects, pages  # noqa: E402


def page_count(pdf):
    try:
        d = open(pdf, "rb").read()
        return len(pages(d, objects(d)))
    except Exception:
        return 0


def buckets(pdf, page):
    try:
        ops = dump(pdf, page)
    except Exception:
        return None
    by = collections.defaultdict(set)
    for font, base, sz, tm, td in ops:
        if sz and td is not None:
            by[round(sz, 1)].add(round(td[1], 3))
    return {k: sorted(v, reverse=True) for k, v in by.items()}


ours_dir, ref_dir = sys.argv[1], sys.argv[2]
only = None
if "--only" in sys.argv:
    only = sys.argv[sys.argv.index("--only") + 1]

print("document\tpaired\tmean|dy|\twithin0.1\twithin1.0")
for o in sorted(glob.glob(os.path.join(ours_dir, "*.pdf"))):
    ident = os.path.basename(o)[:-4]
    if only and only not in ident:
        continue
    r = os.path.join(ref_dir, ident + ".pdf")
    if not os.path.exists(r):
        continue
    n = min(page_count(o), page_count(r))
    tot = 0
    err = 0.0
    close = tight = 0
    for p in range(1, n + 1):
        a, b = buckets(o, p), buckets(r, p)
        if not a or not b:
            continue
        for sz in a:
            if sz not in b or len(a[sz]) != len(b[sz]):
                continue
            for x, y in zip(a[sz], b[sz]):
                d = abs(x - y)
                tot += 1
                err += d
                if d <= 0.1:
                    tight += 1
                if d <= 1.0:
                    close += 1
    if tot:
        print(f"{ident}\t{tot}\t{err / tot:.4f}\t{tight}\t{close}")
