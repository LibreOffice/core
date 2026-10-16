#!/usr/bin/env python3
"""Mechanism evidence for one page of a document pair, from the PDFs' own operators.

    pagemech.py <id> <page> [<page> ...]

Prints, per side: the image list aggregated by dimension, and counts of text
records / glyphs / show operators / fills / glyph-sized fills / images / strokes,
all read out of the content stream via pdf-ops.py.
"""
import collections, pathlib, re, subprocess, sys

S = pathlib.Path("/c/sandbox/workdir/ver-out/sweep")
OPS = "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"

RE_TEXT = re.compile(r"(\d+) glyphs in (\d+) show")
RE_BOX = re.compile(r"\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)")
RE_COL = re.compile(r"#([0-9A-Fa-f]{6})")


def images(pdf, page):
    out = subprocess.run(["pdfimages", "-list", "-f", str(page), "-l", str(page), str(pdf)],
                         capture_output=True, text=True).stdout.splitlines()[2:]
    c = collections.Counter()
    for ln in out:
        f = ln.split()
        if len(f) > 5:
            c[f"{f[2]} {f[3]}x{f[4]}"] += 1
    return c


def ops(pdf, page):
    out = subprocess.run(["python3", OPS, "dump", str(pdf), "--page", str(page)],
                         capture_output=True, text=True).stdout.splitlines()
    st = dict(text=0, glyphs=0, shows=0, fill=0, smallfill=0, image=0, stroke=0)
    fillcol = collections.Counter()
    for ln in out:
        k = ln.split(None, 1)[0] if ln.strip() else ""
        if k == "text":
            st["text"] += 1
            m = RE_TEXT.search(ln)
            if m:
                st["glyphs"] += int(m.group(1)); st["shows"] += int(m.group(2))
        elif k == "fill":
            st["fill"] += 1
            m = RE_BOX.search(ln)
            if m:
                x0, y0, x1, y1 = (float(g) for g in m.groups())
                if 0 < x1 - x0 < 12 and 0 < y1 - y0 < 12:
                    st["smallfill"] += 1
                    mc = RE_COL.search(ln)
                    fillcol[mc.group(1) if mc else "?"] += 1
        elif k == "image":
            st["image"] += 1
        elif k == "stroke":
            st["stroke"] += 1
    return st, fillcol


ident = sys.argv[1]
for page in sys.argv[2:]:
    print(f"### {ident}  page {page}")
    for side in ("ref", "ours"):
        pdf = S / side / f"{ident}.pdf"
        im = images(pdf, page)
        st, fc = ops(pdf, page)
        print(f"  {side:4} text={st['text']:4} glyphs={st['glyphs']:5} shows={st['shows']:5} "
              f"fills={st['fill']:4} glyph-sized-fills={st['smallfill']:4} "
              f"images={st['image']:3} strokes={st['stroke']:4}")
        print(f"       pdfimages: " + (", ".join(f"{k} x{v}" for k, v in im.most_common(6)) or "none"))
        if st["smallfill"]:
            print(f"       small-fill colours: " + ", ".join(f"#{k} x{v}" for k, v in fc.most_common(4)))
