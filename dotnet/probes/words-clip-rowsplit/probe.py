#!/usr/bin/env python3
"""What upper space does a split row's follow part give the paragraph that opens it?

A table row is cut across a page break so that the cut falls *between* two of the cell's
paragraphs. The paragraph that opens the follow part therefore has no paragraph above it on
that page, but does have one in the same cell on the page before. Two rules disagree there:

  * the paragraph re-applies the space-before it was laid out with, which the collapse
    against the previous paragraph's space-after had already taken away; or
  * it re-applies its *own* `w:spacing w:before` in full, because `CalcUpperSpace` finds no
    previous frame in this cell part and never reaches the collapsing branch at all.

They differ exactly when `w:before <= w:after`, which is the commonest shape in real
documents (a style stating the same figure for both).

Measured as the distance from the follow part's own top rule to the first baseline on
page 2, against both installed LibreOffice binaries and, when XRAY_OFFICE_CLI is set, ours.

    python3 probe.py <out-dir>
"""
import os, re, subprocess, sys, html
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx import write

OUT = sys.argv[1]
CLI = os.environ.get("XRAY_OFFICE_CLI")
LO = {"24.2": os.environ.get("LO24", "soffice"),
      "26.2": os.environ.get("LO26", "/opt/libreoffice26.2/program/soffice")}
os.makedirs(OUT, exist_ok=True)


def para(before, after, text):
    return (f'<w:p><w:pPr><w:spacing w:before="{before}" w:after="{after}"/></w:pPr>'
            f'<w:r><w:t xml:space="preserve">{text}</w:t></w:r></w:p>')


def doc(before, after, filler, nparas):
    pre = "".join(para(0, 0, f"Filler line {i}") for i in range(filler))
    inner = "".join(para(before, after, f"Cell paragraph {j}") for j in range(nparas))
    borders = "".join(f'<w:{e} w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
                      for e in ("top", "left", "bottom", "right", "insideH", "insideV"))
    return (pre + '<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/><w:tblBorders>' + borders
            + '</w:tblBorders></w:tblPr><w:tblGrid><w:gridCol w:w="7000"/></w:tblGrid>'
            + '<w:tr><w:tc><w:tcPr><w:tcW w:w="7000" w:type="dxa"/></w:tcPr>'
            + inner + '</w:tc></w:tr></w:tbl><w:p/>')


def render(src, who):
    d = os.path.join(OUT, who.replace(".", ""))
    os.makedirs(d, exist_ok=True)
    pdf = os.path.join(d, os.path.basename(src)[:-5] + ".pdf")
    if os.path.exists(pdf):
        os.remove(pdf)
    if who == "ours":
        subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", d],
                       capture_output=True)
    else:
        subprocess.run([LO[who], f"-env:UserInstallation=file://{d}/prof", "--headless",
                        "--convert-to", "pdf", "--outdir", d, src], capture_output=True)
    return pdf if os.path.exists(pdf) else None


def pagewords(pdf, page):
    x = subprocess.run(["pdftotext", "-f", str(page), "-l", str(page), pdf, "-"],
                       capture_output=True, text=True).stdout
    return x


def firstword(pdf, page):
    x = subprocess.run(["pdftotext", "-bbox", "-f", str(page), "-l", str(page), pdf, "-"],
                       capture_output=True, text=True).stdout
    m = re.search(r'<word xMin="[\d.]+" yMin="([\d.]+)"', x)
    return float(m.group(1)) if m else None


def pages(pdf):
    r = subprocess.run(["pdfinfo", pdf], capture_output=True, text=True).stdout
    m = re.search(r"Pages:\s+(\d+)", r)
    return int(m.group(1)) if m else 0


def toprule(pdf, page):
    """The highest full-width horizontal stroke on the page, in PDF top-down points."""
    txt = subprocess.run(["pdftotext", "-bbox", "-f", str(page), "-l", str(page), pdf, "-"],
                         capture_output=True, text=True).stdout
    m = re.search(r'<page width="([\d.]+)" height="([\d.]+)"', txt)
    height = float(m.group(2)) if m else 792.0
    ops = subprocess.run(["python3", OPS, "dump", pdf, "--page", str(page), "--only", "stroke"],
                         capture_output=True, text=True).stdout
    ys = []
    for line in ops.splitlines():
        g = re.search(r'\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)', line)
        if not g:
            continue
        x1, y1, x2, y2 = map(float, g.groups())
        if abs(y1 - y2) < 0.02 and abs(x2 - x1) > 200:
            ys.append(y1)
    return (height, sorted(ys))


HERE = os.path.dirname(os.path.abspath(__file__))
OPS = os.path.abspath(os.path.join(HERE, "..", "..", "..",
                                   ".claude/skills/render-comparison/scripts/pdf-ops.py"))

WHO = ["24.2", "26.2"] + (["ours"] if CLI else [])
print(f"{'paras':>5} {'before':>7} {'after':>6} {'who':>6} "
      f"{'p2 rule':>8} {'p2 base':>8} {'gap':>7}  split")
for nparas in (4,):
    for before, after in ((240, 0), (0, 240), (240, 240), (240, 120), (120, 240)):
        for who in WHO:
            got = None
            for filler in range(38, 56):
                src = write(os.path.join(OUT, f"n{nparas}b{before}a{after}f{filler}.docx"),
                            doc(before, after, filler, nparas))
                pdf = render(src, who)
                if not pdf or pages(pdf) != 2:
                    continue
                # The row must genuinely be *split*: some of the cell on each page.
                one, two = pagewords(pdf, 1), pagewords(pdf, 2)
                if "Cell paragraph" not in one or "Cell paragraph" not in two:
                    continue
                cut = two.count("Cell paragraph")
                y = firstword(pdf, 2)
                if y is None:
                    continue
                h, rules = toprule(pdf, 2)
                if not rules:
                    continue
                top = h - max(rules)           # highest rule, top-down
                got = (top, y, y - top, cut)
                break
            if got:
                print(f"{nparas:>5} {before:>7} {after:>6} {who:>6} "
                      f"{got[0]:>8.2f} {got[1]:>8.2f} {got[2]:>7.2f}  "
                      f"{got[3]} of {nparas} paragraphs on page 2")
            else:
                print(f"{nparas:>5} {before:>7} {after:>6} {who:>6}   (no split found)")
