#!/usr/bin/env python3
r"""Both renderings of every probe, scored on the rule each applied.

    compareab.py <outdir>

A probe is read as *max* (the word honoured) or *sum* (the word too late) from
the distance between its AAAA and BBBB marks, which the probe families set to
24 pt of `\sa` against 24 pt of `\sb`.  The midpoint of the two answers is the
discriminator, so it does not depend on the face's line height.
"""
import sys, pathlib, pymupdf

OUT = pathlib.Path(sys.argv[1])

def ab(pdf):
    doc = pymupdf.open(pdf)
    got = {}
    for page in doc:
        for b in page.get_text("dict")["blocks"]:
            if b["type"]:
                continue
            for l in b["lines"]:
                t = "".join(s["text"] for s in l["spans"]).strip()
                for k in ("AAAA", "BBBB"):
                    if t.startswith(k) and k not in got:
                        got[k] = l["bbox"][1]
    doc.close()
    return None if len(got) < 2 else round(got["BBBB"] - got["AAAA"], 2)

def rule(v):
    return "?" if v is None else ("max" if v < 50 else "sum")

print(f"{'probe':24s} {'ref':>8s} {'ours':>8s}  {'ref':>4s} {'ours':>4s}  agree")
agree = total = 0
for f in sorted((OUT / "ref").glob("*.pdf")):
    r = ab(f)
    o = ab(OUT / "ours" / f.name) if (OUT / "ours" / f.name).exists() else None
    ok = r is not None and o is not None and rule(r) == rule(o)
    total += 1
    agree += ok
    print(f"{f.stem:24s} {str(r):>8s} {str(o):>8s}  {rule(r):>4s} {rule(o):>4s}  {'yes' if ok else 'NO'}")
print(f"\nagree {agree} of {total}")
