#!/usr/bin/env python3
"""How much rotated text does the reference draw that we draw upright?

**The obvious version of this is wrong and its first cut was.** LibreOffice rotates text by
writing a rotated `Tm` inside `BT ... ET`; X-Ray.Office rotates it by pushing a rotated `cm` on
the graphics state and then writing an upright `Tm` (or none at all). A census that looks only
at `Tm` therefore reports that we draw *no* rotated text anywhere — 18832 operators to nought —
when in fact we rotate through the other operator on many pages. On
`NAS-Infrastructure-Roadmaps-v16.0.pptx` page 8 we write 5 rotated `cm` against the reference's
77 rotated `Tm`, and the two renderings' rotated captions are visually the same.

So this counts, per page, TEXT-DRAWING BLOCKS THAT ARE ROTATED BY EITHER ROUTE: a `BT` whose
own `Tm` is a rotation, or a `BT` reached while a rotated `cm` is in effect. `q`/`Q` is
tracked so a rotation that has been popped does not leak.

    rotated-text-census.py <ours-dir> <ref-dir> [ext]
"""
import collections, glob, os, re, sys
sys.path.insert(0, "/c/sandbox/workdir/wt-slides-r50/dotnet/research/probes/slides-r15")
import pdfops  # noqa: E402
from pdfops import objects, pages  # noqa: E402

TOKEN = re.compile(
    rb"(?:(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(cm|Tm))"
    rb"|\b(q|Q|BT)\b")


def blocks(stream):
    """(rotated text blocks, total text blocks) in one content stream."""
    stack = [False]          # is a rotation in effect?
    rotated_tm = False
    rot = tot = 0
    for m in TOKEN.finditer(stream):
        op = m.group(7) or m.group(8)
        if op == b"q":
            stack.append(stack[-1])
        elif op == b"Q":
            if len(stack) > 1:
                stack.pop()
        elif op == b"BT":
            tot += 1
            rotated_tm = False
            if stack[-1]:
                rot += 1
        elif op == b"cm":
            b, c = float(m.group(2)), float(m.group(3))
            if abs(b) > 1e-6 or abs(c) > 1e-6:
                stack[-1] = True
        elif op == b"Tm":
            b, c = float(m.group(2)), float(m.group(3))
            if (abs(b) > 1e-6 or abs(c) > 1e-6) and not stack[-1] and not rotated_tm:
                rot += 1
                rotated_tm = True
    return rot, tot


def streams(path):
    data = open(path, "rb").read()
    objs = objects(data)
    for page in pages(data, objs):
        yield pdfops.content(data, objs, page)


if __name__ == "__main__":
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    want = sys.argv[3] if len(sys.argv) > 3 else None

    rows = []
    tot_o = tot_r = 0
    pages_missing = 0
    for pdf in sorted(glob.glob(os.path.join(ref_dir, "*.pdf"))):
        ident = os.path.basename(pdf)[:-4]
        if want and not ident.endswith("__" + want):
            continue
        ours = os.path.join(ours_dir, ident + ".pdf")
        if not os.path.exists(ours):
            continue
        try:
            a = [blocks(s) for s in streams(ours)]
            b = [blocks(s) for s in streams(pdf)]
        except Exception as exc:
            print(f"  !! {ident}: {exc}")
            continue
        ro = sum(x for x, _ in a)
        rr = sum(x for x, _ in b)
        miss = sum(1 for i in range(min(len(a), len(b))) if b[i][0] and not a[i][0])
        tot_o += ro
        tot_r += rr
        pages_missing += miss
        if ro or rr:
            rows.append((rr - ro, ident, ro, rr, miss))

    rows.sort(key=lambda t: -t[0])
    print(f"{'ref-ours':>9} {'ours':>7} {'ref':>7} {'ref-only pages':>15}  document")
    for d, ident, ro, rr, miss in rows:
        print(f"{d:9d} {ro:7d} {rr:7d} {miss:15d}  {ident}")
    print(f"\ndocuments drawing rotated text on either side: {len(rows)}")
    print(f"rotated text blocks: ours {tot_o}, reference {tot_r}")
    print(f"pages where the reference rotates text and we rotate none: {pages_missing}")
