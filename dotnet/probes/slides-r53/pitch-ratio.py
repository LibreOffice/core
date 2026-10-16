#!/usr/bin/env python3
"""What line-spacing scales does the reference actually draw, per format?

`constScaleLevels` pairs every font scale with a spacing scale of 0.900 or 0.800, so a
shrunk body should show a baseline pitch of 1.08 or 0.96 ems.  This asks the reference's own
renderings which ratios it draws, split by the format the document came from.

Per page: group the text-showing operators by /Tf size, keep any group of three or more
baselines with a constant pitch, and record pitch / size.

    pitch-ratio.py <ref-dir> <corpus-root>
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


def ratios(pdf, page):
    try:
        ops = dump(pdf, page)
    except Exception:
        return []
    by = collections.defaultdict(list)
    for font, base, sz, tm, td in ops:
        if sz and td is not None:
            by[round(sz, 3)].append(round(td[1], 3))
    out = []
    for sz, ys in by.items():
        ys = sorted(set(ys), reverse=True)
        if len(ys) < 3:
            continue
        gaps = [round(ys[i] - ys[i + 1], 3) for i in range(len(ys) - 1)]
        # the longest run of a constant gap, so a page holding two blocks still contributes
        best, run, cur = 0, 1, None
        for g in gaps:
            if cur is not None and abs(g - cur) < 0.05:
                run += 1
            else:
                cur, run = g, 1
            if run > best:
                best, keep = run, cur
        if best >= 2:
            out.append((sz, round(keep / sz, 4)))
    return out


ref_dir = sys.argv[1]
hist = collections.defaultdict(collections.Counter)
for pdf in sorted(glob.glob(os.path.join(ref_dir, "*.pdf"))):
    ident = os.path.basename(pdf)[:-4]
    ext = ident.rsplit("__", 1)[-1]
    n = page_count(pdf)
    for p in range(1, n + 1):
        for sz, r in ratios(pdf, p):
            hist[ext][round(r, 2)] += 1

for ext in sorted(hist):
    total = sum(hist[ext].values())
    print(f"== {ext}: {total} constant-pitch blocks")
    for r, c in sorted(hist[ext].items(), key=lambda t: -t[1])[:14]:
        print(f"   ratio {r:6.2f}  {c:6d}  {100.0 * c / total:5.1f}%")
