#!/usr/bin/env python3
"""Read AAAA/BBBB/CCCC out of both renderings of each probe and table the three numbers.

    measure.py <outdir-from-render.sh>
"""
import sys, pathlib, pymupdf

OUT = pathlib.Path(sys.argv[1])

def marks(pdf):
    doc = pymupdf.open(pdf)
    got = {}
    for pno, page in enumerate(doc):
        for b in page.get_text("dict")["blocks"]:
            if b["type"]: continue
            for l in b["lines"]:
                t = "".join(s["text"] for s in l["spans"]).strip()
                for k in ("AAAA", "BBBB", "CCCC"):
                    if t.startswith(k) and k not in got:
                        got[k] = (pno + 1, round(l["bbox"][0], 2), round(l["bbox"][1], 2))
    return got

print(f"{'probe':22s} {'side':5s} {'x(B)':>8s} {'A->B':>8s} {'B->C':>8s}")
agree = total = 0
for f in sorted(OUT.glob("ref/*.pdf")):
    name = f.stem
    row = {}
    for side in ("ref", "ours"):
        p = OUT / side / f"{name}.pdf"
        if not p.exists():
            row[side] = None
            continue
        m = marks(p)
        if not all(k in m for k in ("AAAA", "BBBB", "CCCC")):
            row[side] = None
            continue
        row[side] = (m["BBBB"][1], round(m["BBBB"][2] - m["AAAA"][2], 2),
                     round(m["CCCC"][2] - m["BBBB"][2], 2))
    for side in ("ref", "ours"):
        v = row[side]
        print(f"{name:22s} {side:5s} " + ("  (missing)" if v is None
              else f"{v[0]:8.2f} {v[1]:8.2f} {v[2]:8.2f}"))
    total += 1
    if row["ref"] and row["ours"] and all(abs(a - b) < 0.6 for a, b in zip(row["ref"], row["ours"])):
        agree += 1
    print()
print(f"agree {agree} of {total}")
