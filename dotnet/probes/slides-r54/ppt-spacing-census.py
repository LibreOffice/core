#!/usr/bin/env python3
"""Where do we draw a shrunk baseline pitch that the reference draws unshrunk?

`constScaleLevels` pairs every autofit font scale with a spacing scale of 0.900 or 0.800, so
a fitted body drawn with the spacing applied has a baseline pitch of 1.08 or 0.96 ems and one
drawn without has 1.20.  This pairs our renderings against the reference's, block for block,
and counts the blocks where the two disagree in exactly that way.

It is a census of the RENDERING, not of the markup, which is the point: whether a fit is
solved at all, and which level it lands on, are properties of the layout and invisible in the
record.  What it cannot see is stated in the round's prediction file -- chiefly that a block
whose font scale ALSO differs may be counted as a spacing disagreement, and that a page whose
two sides disagree on line breaking produces no comparable block at all.

    ppt-spacing-census.py <ours-dir> <ref-dir> [ext]
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


def blocks(pdf, page):
    """{rounded /Tf size: (pitch/size, baselines)} for every constant-pitch block."""
    try:
        ops = dump(pdf, page)
    except Exception:
        return {}
    by = collections.defaultdict(list)
    for font, base, sz, tm, td in ops:
        if sz and td is not None:
            by[round(sz, 1)].append(round(td[1], 3))
    out = {}
    for sz, ys in by.items():
        ys = sorted(set(ys), reverse=True)
        if len(ys) < 3:
            continue
        gaps = [round(ys[i] - ys[i + 1], 3) for i in range(len(ys) - 1)]
        best, run, cur, keep = 0, 1, None, None
        for g in gaps:
            if cur is not None and abs(g - cur) < 0.05:
                run += 1
            else:
                cur, run = g, 1
            if run > best:
                best, keep = run, cur
        if best >= 2 and keep:
            out[sz] = (round(keep / sz, 4), len(ys))
    return out


def near(x, v, tol=0.02):
    return abs(x - v) < tol


if __name__ == "__main__":
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    want = sys.argv[3] if len(sys.argv) > 3 else None

    per_doc = collections.Counter()
    per_doc_pages = collections.defaultdict(set)
    total_blocks = shrunk_here = 0
    hist = collections.Counter()

    for pdf in sorted(glob.glob(os.path.join(ref_dir, "*.pdf"))):
        ident = os.path.basename(pdf)[:-4]
        ext = ident.rsplit("__", 1)[-1]
        if want and ext != want:
            continue
        ours = os.path.join(ours_dir, ident + ".pdf")
        if not os.path.exists(ours):
            continue
        n = min(page_count(pdf), page_count(ours))
        for p in range(1, n + 1):
            rb, ob = blocks(pdf, p), blocks(ours, p)
            for sz in set(rb) & set(ob):
                rr, _ = rb[sz]
                orr, _ = ob[sz]
                total_blocks += 1
                hist[(round(orr, 2), round(rr, 2))] += 1
                if (near(orr, 0.96) or near(orr, 1.08)) and near(rr, 1.20):
                    shrunk_here += 1
                    per_doc[ident] += 1
                    per_doc_pages[ident].add(p)

    print(f"comparable constant-pitch blocks : {total_blocks}")
    print(f"ours shrunk, reference not       : {shrunk_here}"
          f"  in {len(per_doc)} documents, {sum(len(v) for v in per_doc_pages.values())} pages")
    for ident, c in per_doc.most_common():
        print(f"   {c:4d} blocks  {len(per_doc_pages[ident]):3d} pages  {ident}")
    print("\ntop (ours, ref) pitch-ratio pairs")
    for (o, r), c in hist.most_common(12):
        print(f"   ours {o:5.2f}  ref {r:5.2f}  {c:6d}")
