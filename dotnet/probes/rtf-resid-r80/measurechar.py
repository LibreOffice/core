#!/usr/bin/env python3
"""The one paragraph's size, width and face, both ways, against each side's own control."""
import sys
import pathlib
import pymupdf

OUT = pathlib.Path(sys.argv[1])


def read(pdf):
    doc = pymupdf.open(pdf)
    for b in doc[0].get_text("dict")["blocks"]:
        if b["type"]:
            continue
        for line in b["lines"]:
            t = "".join(s["text"] for s in line["spans"])
            if t.strip().startswith("BBBB"):
                s = line["spans"][0]
                return (round(s["size"], 2), round(line["bbox"][2] - line["bbox"][0], 2),
                        s["font"].split("+")[-1])
    return None


rows = {f.stem: {side: read(OUT / side / f"{f.stem}.pdf") for side in ("ref", "ours")}
        for f in sorted(OUT.glob("ref/*.pdf"))}
base = rows["control-none"]
print(f"{'probe':16s} {'ref':38s} {'ours':38s}  same?")
agree = 0
for name, r in rows.items():
    def show(side):
        v, b = r[side], base[side]
        if v is None:
            return "(missing)".ljust(38)
        mark = "".join(c for c, i in (("S", 0), ("W", 1), ("F", 2)) if v[i] != b[i])
        return f"{v[0]:6.2f} {v[1]:7.2f} {v[2]:<20s} {mark:3s}"
    ok = (r["ref"] and r["ours"]
          and (r["ref"][0] == base["ref"][0]) == (r["ours"][0] == base["ours"][0])
          and (r["ref"][2] == base["ref"][2]) == (r["ours"][2] == base["ours"][2])
          and abs((r["ref"][1] - base["ref"][1]) - (r["ours"][1] - base["ours"][1])) < 3)
    agree += 1 if ok else 0
    print(f"{name:16s} {show('ref')} {show('ours')}  {'' if ok else 'DIFFER'}")
print(f"\nagree {agree} of {len(rows)}")
