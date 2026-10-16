#!/usr/bin/env python3
"""How tall are the two parts of a table row that LibreOffice splits across a page?

Every number is read out of the reference PDF's own horizontal rules — the row's borders —
so a part's height is measured where the renderer drew it rather than inferred from its text.

The question this settles: our follow part is 1.00 pt shorter than 26.2.4.2's on
`Sample_SQMS_Program.docx`, and two mechanisms predict exactly that difference.

  (i)  the follow part re-applies the split paragraph's `w:spacing w:before`
  (ii) a *sibling* cell whose own content fitted entirely on the first part is
       nevertheless re-laid-out on the follow part, contributing a whole empty line

Variant `solo` has no sibling cell at all, so it separates them: under (i) its follow part
carries the space-before, under (ii) it does not.
"""
import os, subprocess, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx import write

OPS = "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
OUT = sys.argv[1]
CLI = os.environ.get("XRAY_OFFICE_CLI")
os.makedirs(OUT, exist_ok=True)

# 12 pt Liberation Serif => 13.80 pt a line. Filler paragraphs push the table down so the
# row lands on the page boundary; `filler` is swept until the row actually splits.
def GRIDOF(variant):
    cols = {"solo": [7000], "label": [1200, 7000], "empty": [7000, 600],
            "both": [1200, 7000, 600]}[variant]
    return "<w:tblGrid>" + "".join(f'<w:gridCol w:w="{c}"/>' for c in cols) + "</w:tblGrid>"
LINE = "Alpha bravo charlie delta echo foxtrot golf hotel india juliet kilo lima mike november"

def cell(w, paras):
    return f'<w:tc><w:tcPr><w:tcW w:w="{w}" w:type="dxa"/></w:tcPr>{paras}</w:tc>'

def para(before, after, text, bold=False):
    b = '<w:b/>' if bold else ''
    return (f'<w:p><w:pPr><w:spacing w:before="{before}" w:after="{after}"/>'
            f'<w:rPr>{b}</w:rPr></w:pPr>'
            + (f'<w:r><w:rPr>{b}</w:rPr><w:t xml:space="preserve">{text}</w:t></w:r>' if text else '')
            + '</w:p>')

def table(variant):
    body = cell(7000, para(20, 20, LINE + " " + LINE))          # wraps to 2 lines
    if variant == "solo":
        cells = body
    elif variant == "label":
        cells = cell(1200, para(40, 40, "L.1.")) + body
    elif variant == "empty":
        cells = body + cell(600, para(20, 20, ""))
    else:  # both
        cells = cell(1200, para(40, 40, "L.1.")) + body + cell(600, para(20, 20, ""))
    return ('<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/>'
            '<w:tblBorders>'
            + "".join(f'<w:{e} w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
                      for e in ("top", "left", "bottom", "right", "insideH", "insideV"))
            + '</w:tblBorders></w:tblPr>'
            + GRIDOF(variant)
            + '<w:tr><w:trPr><w:trHeight w:val="270"/></w:trPr>'
            + cell(1200, para(40, 40, "K.0.")) + cell(7000, para(20, 20, "Anchor row"))
            + ('' if variant in ("solo", "label") else cell(600, para(20, 20, "")))
            + '</w:tr>'
            '<w:tr><w:trPr><w:trHeight w:val="270"/></w:trPr>' + cells + '</w:tr>'
            '</w:tbl><w:p/>')

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

def render(src, outdir, use_cli):
    os.makedirs(outdir, exist_ok=True)
    if use_cli:
        subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", outdir],
                       capture_output=True)
    else:
        subprocess.run(["soffice", "-env:UserInstallation=file://" + OUT + "/prof",
                        "--headless", "--convert-to", "pdf", "--outdir", outdir, src],
                       capture_output=True)
    return os.path.join(outdir, os.path.basename(src)[:-5] + ".pdf")

print(f"{'variant':>8} {'fill':>5} {'who':>4} {'pages':>5}  part1 (top->rule)      part2 (top->rule)")
for variant in ("solo", "label", "empty", "both"):
    for filler in (44,):
        pre = "".join(para(0, 0, f"Filler line {i}") for i in range(filler))
        src = write(os.path.join(OUT, f"{variant}-{filler}.docx"), pre + table(variant))
        for who, use_cli in (("ref", False), ("ours", True)) if CLI else (("ref", False),):
            pdf = render(src, os.path.join(OUT, who), use_cli)
            n = int(subprocess.run(["pdfinfo", pdf], capture_output=True, text=True)
                    .stdout.split("Pages:")[1].split()[0])
            r1 = rules(pdf, 1)
            r2 = rules(pdf, 2) if n > 1 else []
            p1 = f"{r1[-2]:.2f}->{r1[-1]:.2f} = {r1[-2]-r1[-1]:6.2f}" if len(r1) >= 2 else "-"
            p2 = f"{r2[0]:.2f}->{r2[1]:.2f} = {r2[0]-r2[1]:6.2f}" if len(r2) >= 2 else "-"
            print(f"{variant:>8} {filler:>5} {who:>4} {n:>5}  {p1}   {p2}")
