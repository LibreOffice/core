#!/usr/bin/env python3
"""Which of the split paragraph's two spacings does LibreOffice's follow part carry?

One cell, one paragraph, two lines, split across a page. `w:spacing w:before` and
`w:spacing w:after` are swept independently, so the follow part's height names the term
directly instead of leaving before+after summed and ambiguous.

The filler count is swept too, because the row only splits when it lands on the boundary,
and the two renderers reach the boundary at different filler counts.
"""
import os, subprocess, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx import write

OPS = "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
OUT = sys.argv[1]
CLI = os.environ.get("XRAY_OFFICE_CLI")
os.makedirs(OUT, exist_ok=True)
GRID = '<w:tblGrid><w:gridCol w:w="7000"/></w:tblGrid>'
LINE = "Alpha bravo charlie delta echo foxtrot golf hotel india juliet kilo lima mike november"

def para(before, after, text):
    return (f'<w:p><w:pPr><w:spacing w:before="{before}" w:after="{after}"/></w:pPr>'
            + (f'<w:r><w:t xml:space="preserve">{text}</w:t></w:r>' if text else '') + '</w:p>')

def doc(before, after, filler):
    pre = "".join(para(0, 0, f"Filler line {i}") for i in range(filler))
    tbl = ('<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/><w:tblBorders>'
           + "".join(f'<w:{e} w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
                     for e in ("top", "left", "bottom", "right", "insideH", "insideV"))
           + '</w:tblBorders></w:tblPr>' + GRID
           + '<w:tr><w:tc><w:tcPr><w:tcW w:w="7000" w:type="dxa"/></w:tcPr>'
           + para(0, 0, "Anchor row") + '</w:tc></w:tr>'
           '<w:tr><w:tc><w:tcPr><w:tcW w:w="7000" w:type="dxa"/></w:tcPr>'
           + para(before, after, LINE + " " + LINE) + '</w:tc></w:tr></w:tbl><w:p/>')
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
    return round(r1[-2] - r1[-1], 2), round(r2[0] - r2[1], 2)

CASES = [(0, 0), (20, 0), (0, 20), (20, 20), (40, 0), (0, 40), (40, 40), (100, 60)]
print(f"{'before':>7} {'after':>6} | {'ref part1':>10} {'ref part2':>10} | {'our part1':>10} {'our part2':>10}")
for before, after in CASES:
    got = {}
    for who in (("ref", "ours") if CLI else ("ref",)):
        for filler in range(40, 52):
            src = write(os.path.join(OUT, f"b{before}a{after}f{filler}.docx"),
                        doc(before, after, filler))
            m = measure(src, who)
            if m:
                got[who] = m
                break
    r = got.get("ref"); o = got.get("ours")
    print(f"{before:>7} {after:>6} | {(r[0] if r else 0):>10.2f} {(r[1] if r else 0):>10.2f} |"
          f" {(o[0] if o else 0):>10.2f} {(o[1] if o else 0):>10.2f}")
