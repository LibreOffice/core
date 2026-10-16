#!/usr/bin/env python3
"""Read the five smaller families of `genextra.py` out of both renderings.

    measureextra.py <outdir-from-render.sh>

Each family gets the reading that can see it: `chain-*` and `sum-*` and `htm-*` are read as
BBBB's x and its distance from AAAA, `deff-*` as the face BBBB is set in, `keep*` as the page
BBBB lands on.
"""
import sys
import pathlib
import pymupdf


def lines(pdf):
    doc = pymupdf.open(pdf)
    out = []
    for pno, page in enumerate(doc):
        for b in page.get_text("dict")["blocks"]:
            if b["type"]:
                continue
            for line in b["lines"]:
                t = "".join(s["text"] for s in line["spans"]).strip()
                if t:
                    out.append((pno + 1, t, line["bbox"], line["spans"][0]))
    return len(doc), out


def reading(pdf):
    pages, ls = lines(pdf)
    b = next((x for x in ls if x[1].startswith("BBBB")), None)
    a = next((x for x in ls if x[1].startswith("AAAA")), None)
    if b is None:
        return None
    if pdf.stem.startswith("deff-"):
        return ("face", b[3]["font"].split("+")[-1])
    if pdf.stem.startswith("keep"):
        c = next((x for x in ls if x[1].startswith("CCCC")), None)
        return ("page", f"pages={pages} BBBB=p{b[0]} CCCC=p{c[0] if c else '?'}")
    if a is None:
        return None
    return ("xy", round(b[2][0], 2), round(b[2][1] - a[2][1], 2))


def agrees(a, b):
    """The two renderings put a left margin 0.10 pt apart on every probe, control included, so
    the geometric readings are compared at a tolerance and the categorical ones exactly."""
    if a is None or b is None or a[0] != b[0]:
        return False
    if a[0] != "xy":
        return a == b
    return all(abs(x - y) < 0.5 for x, y in zip(a[1:], b[1:]))


OUT = pathlib.Path(sys.argv[1])
agree = total = 0
for f in sorted(OUT.glob("ref/*.pdf")):
    r = {side: reading(OUT / side / f.name) for side in ("ref", "ours")}
    total += 1
    same = agrees(r["ref"], r["ours"])
    agree += 1 if same else 0
    print(f"{f.stem:24s} ref {str(r['ref']):46s} ours {str(r['ours']):46s}"
          + ("" if same else "  DIFFER"))
print(f"\nagree {agree} of {total}")
