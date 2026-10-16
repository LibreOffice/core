#!/usr/bin/env python3
r"""Read AAAA/BBBB/CCCC out of each rendering and report the A->B distance.

    measureab.py <outdir> [ref|ours]

A->B near the sum of the two adjacent spacings is `\htmautsp` arriving too late;
near the larger of them is the word honoured.
"""
import sys, pathlib, pymupdf

OUT = pathlib.Path(sys.argv[1])
SIDE = sys.argv[2] if len(sys.argv) > 2 else "ref"

def marks(pdf):
    doc = pymupdf.open(pdf)
    got = {}
    for pno, page in enumerate(doc):
        for b in page.get_text("dict")["blocks"]:
            if b["type"]:
                continue
            for l in b["lines"]:
                t = "".join(s["text"] for s in l["spans"]).strip()
                for k in ("AAAA", "BBBB", "CCCC"):
                    if t.startswith(k) and k not in got:
                        got[k] = (pno + 1, round(l["bbox"][0], 2), round(l["bbox"][1], 2))
    doc.close()
    return got

print(f"{'probe':>14s} {'A->B':>8s} {'B->C':>8s}  rule")
for f in sorted((OUT / SIDE).glob("*.pdf")):
    m = marks(f)
    if not all(k in m for k in ("AAAA", "BBBB", "CCCC")):
        print(f"{f.stem:>14s}   (marks found: {sorted(m)} -- unreadable)")
        continue
    ab = round(m["BBBB"][2] - m["AAAA"][2], 2)
    bc = round(m["CCCC"][2] - m["BBBB"][2], 2)
    # The two answers are 24 pt apart whatever the face's line height, so the
    # midpoint of the band separates them without depending on it.
    v = "max" if ab < 50 else "sum"
    print(f"{f.stem:>14s} {ab:8.2f} {bc:8.2f}  {v}")
