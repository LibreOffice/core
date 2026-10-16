#!/usr/bin/env python3
"""Read the seven numbers of each own/inherited probe out of both renderings.

    measurematrix.py <outdir-from-render.sh>

Columns: x1 (BBBB line 1's left), x2 (line 2's left), right (BBBB's widest right edge),
sb (AAAA -> BBBB), sa (BBBB's last line -> CCCC), pitch (line 1 -> line 2).
"""
import sys
import pathlib
import pymupdf

OUT = pathlib.Path(sys.argv[1])
KEYS = ("x1", "x2", "right", "sb", "sa", "pitch")


def read(pdf: pathlib.Path):
    doc = pymupdf.open(pdf)
    lines = []
    for page in doc:
        for b in page.get_text("dict")["blocks"]:
            if b["type"]:
                continue
            for line in b["lines"]:
                t = "".join(s["text"] for s in line["spans"]).strip()
                if t:
                    lines.append((t, line["bbox"]))
    a = next((bb for t, bb in lines if t.startswith("AAAA")), None)
    c = next((bb for t, bb in lines if t.startswith("CCCC")), None)
    bs = [bb for t, bb in lines if not t.startswith("AAAA") and not t.startswith("CCCC")]
    if a is None or c is None or len(bs) < 2:
        return None
    return {
        "x1": round(bs[0][0], 2),
        "x2": round(bs[1][0], 2),
        "right": round(max(bb[2] for bb in bs), 2),
        "sb": round(bs[0][1] - a[1], 2),
        "sa": round(c[1] - bs[-1][1], 2),
        "pitch": round(bs[1][1] - bs[0][1], 2),
    }


rows = {}
for f in sorted(OUT.glob("ref/*.pdf")):
    rows[f.stem] = {side: read(OUT / side / f"{f.stem}.pdf") for side in ("ref", "ours")}

# Each side is compared against ITS OWN control, so a constant that both carry -- the ragged
# right edge of a wrapped line is 2.8 pt apart between the two renderings on every probe,
# including the control -- cannot be read as a difference this round's rule causes.
base = {side: rows["control-none"][side] for side in ("ref", "ours")}


def moved(v, side):
    b = base[side]
    return {k: round(v[k] - b[k], 2) for k in KEYS} if v and b else None


print(f"{'probe':18s} {'side':5s} " + " ".join(f"{k:>8s}" for k in KEYS)
      + "     <- change from that side's own control")
agree = 0
for name, r in rows.items():
    d = {side: moved(r[side], side) for side in ("ref", "ours")}
    for side in ("ref", "ours"):
        if d[side] is None:
            print(f"{name:18s} {side:5s}   (missing)")
            continue
        print(f"{name:18s} {side:5s} " + " ".join(f"{d[side][k]:8.2f}" for k in KEYS))
    if d["ref"] and d["ours"] and all(abs(d["ref"][k] - d["ours"][k]) < 0.75 for k in KEYS):
        agree += 1
    else:
        print(f"{'':18s} {'DIFFER':5s}")
print(f"\nagree {agree} of {len(rows)}")
