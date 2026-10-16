#!/usr/bin/env python3
"""Does a follow part that begins a *fresh* paragraph carry that paragraph's space-before?

`probe-rowsplit-spacing.py` establishes that a follow part which continues a paragraph
mid-way carries the paragraph's `w:spacing w:before` all the same. That leaves the general
form of the rule open: "every part of a split paragraph carries its space-before" and "the
top of a follow part is always the containing block's top" agree on that case and disagree
here, where the cut falls *between* two paragraphs.

Two paragraphs of one line each, cut between them, space-before swept.
"""
import os, subprocess, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx import write

OPS = "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
OUT = sys.argv[1]
CLI = os.environ.get("XRAY_OFFICE_CLI")
os.makedirs(OUT, exist_ok=True)
GRID = '<w:tblGrid><w:gridCol w:w="7000"/></w:tblGrid>'

def para(before, after, text):
    return (f'<w:p><w:pPr><w:spacing w:before="{before}" w:after="{after}"/></w:pPr>'
            + (f'<w:r><w:t xml:space="preserve">{text}</w:t></w:r>' if text else '') + '</w:p>')

def doc(before, after, filler, nparas):
    pre = "".join(para(0, 0, f"Filler line {i}") for i in range(filler))
    inner = "".join(para(before, after, f"Cell paragraph {j}") for j in range(nparas))
    tbl = ('<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/><w:tblBorders>'
           + "".join(f'<w:{e} w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
                     for e in ("top", "left", "bottom", "right", "insideH", "insideV"))
           + '</w:tblBorders></w:tblPr>' + GRID
           + '<w:tr><w:tc><w:tcPr><w:tcW w:w="7000" w:type="dxa"/></w:tcPr>'
           + para(0, 0, "Anchor row") + '</w:tc></w:tr>'
           + '<w:tr><w:tc><w:tcPr><w:tcW w:w="7000" w:type="dxa"/></w:tcPr>'
           + inner + '</w:tc></w:tr></w:tbl><w:p/>')
    return pre + tbl

def rules(pdf, page):
    out = subprocess.run(["python3", OPS, "dump", pdf, "--page", str(page), "--only", "stroke"],
                         capture_output=True, text=True).stdout
    ys = []
    for l in out.splitlines():
        m = re.search(r'\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)', l)
        if not m:
            continue
        x1, y1, x2, y2 = map(float, m.groups())
        if abs(y1 - y2) < 0.01 and x2 - x1 > 200:
            ys.append(round(y1, 2))
    return sorted(set(ys), reverse=True)

def measure(src, who):
    outdir = os.path.join(OUT, who)
    os.makedirs(outdir, exist_ok=True)
    if who == "ours":
        subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", outdir],
                       capture_output=True)
    else:
        subprocess.run(["soffice", "-env:UserInstallation=file://" + OUT + "/prof",
                        "--headless", "--convert-to", "pdf", "--outdir", outdir, src],
                       capture_output=True)
    pdf = os.path.join(outdir, os.path.basename(src)[:-5] + ".pdf")
    if not os.path.exists(pdf):
        return None
    n = int(subprocess.run(["pdfinfo", pdf], capture_output=True, text=True)
            .stdout.split("Pages:")[1].split()[0])
    if n < 2:
        return None
    r1, r2 = rules(pdf, 1), rules(pdf, 2)
    if len(r1) < 2 or len(r2) < 2:
        return None
    # the part's own top edge is its highest rule on that page, its bottom the lowest
    return round(r1[-2] - r1[-1], 2), round(r2[0] - r2[-1], 2)

print(f"{'paras':>5} {'before':>7} {'after':>6} | {'ref p1':>8} {'ref p2':>8} | {'our p1':>8} {'our p2':>8}")
for nparas in (2, 3):
    for before, after in ((0, 0), (40, 0), (0, 40), (40, 40)):
        got = {}
        for who in (("ref", "ours") if CLI else ("ref",)):
            for filler in range(38, 54):
                src = write(os.path.join(OUT, f"n{nparas}b{before}a{after}f{filler}.docx"),
                            doc(before, after, filler, nparas))
                m = measure(src, who)
                if m:
                    got[who] = m
                    break
        r = got.get("ref"); o = got.get("ours")
        print(f"{nparas:>5} {before:>7} {after:>6} | {(r[0] if r else 0):>8.2f} {(r[1] if r else 0):>8.2f} |"
              f" {(o[0] if o else 0):>8.2f} {(o[1] if o else 0):>8.2f}")
