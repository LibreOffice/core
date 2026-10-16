#!/usr/bin/env python3
"""LibreOffice's ascent and line height, size by size, read off the page.

`probe-lineheight.py` shows the two stacks differ by exactly one twip on 21 of 195 (face, size)
pairs and never by more, but a baseline-to-baseline gap alone does not say *which* of the three
terms rounds differently. One page per size puts the first baseline at a known distance below the
top margin, so the ascent is directly measurable and the descent falls out of the difference.
"""
import os, subprocess, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx import write

OPS = "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
OUT = sys.argv[1]
CLI = os.environ.get("XRAY_OFFICE_CLI")
os.makedirs(OUT, exist_ok=True)

FONTS = ["Liberation Serif", "Liberation Sans", "Carlito", "Caladea", "DejaVu Sans"]
SIZES = list(range(10, 49))          # half-points, 5.0 .. 24.0 pt
TOP = 720.0                          # body top: 792 pt page less a 72 pt margin

body = []
for fi, fam in enumerate(FONTS):
    for hp in SIZES:
        rpr = f'<w:rFonts w:ascii="{fam}" w:hAnsi="{fam}"/><w:sz w:val="{hp}"/><w:szCs w:val="{hp}"/>'
        for i in range(2):
            brk = '<w:r><w:br w:type="page"/></w:r>' if (i == 0 and body) else ''
            body.append(f'<w:p><w:pPr><w:rPr>{rpr}</w:rPr></w:pPr>{brk}'
                        f'<w:r><w:rPr>{rpr}</w:rPr><w:t>F{fi}S{hp}L{i}</w:t></w:r></w:p>')

src = write(os.path.join(OUT, "ascent.docx"), "".join(body))

def render(who):
    d = os.path.join(OUT, who)
    os.makedirs(d, exist_ok=True)
    if who == "ours":
        subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", d], capture_output=True)
    else:
        subprocess.run(["soffice", "-env:UserInstallation=file://" + OUT + "/prof", "--headless",
                        "--convert-to", "pdf", "--outdir", d, src], capture_output=True)
    return os.path.join(d, "ascent.pdf")

def read(pdf):
    got = {}
    n = int(subprocess.run(["pdfinfo", pdf], capture_output=True, text=True)
            .stdout.split("Pages:")[1].split()[0])
    for p in range(1, n + 1):
        out = subprocess.run(["python3", OPS, "dump", pdf, "--page", str(p), "--only", "text"],
                             capture_output=True, text=True).stdout
        rows = []
        for l in out.splitlines():
            m = re.search(r'\(\s*([-\d.]+),\s*([-\d.]+)\).*?"?(F\d+S\d+)L(\d)', l)
            if m:
                rows.append((m.group(3), int(m.group(4)), float(m.group(2))))
        keys = {r[0] for r in rows}
        for k in keys:
            ys = {r[1]: r[2] for r in rows if r[0] == k}
            if 0 in ys and 1 in ys:
                got[k] = (round(TOP - ys[0], 3), round(ys[0] - ys[1], 3))
    return got

ref = read(render("ref"))
ours = read(render("ours")) if CLI else {}
print(f"{'face':>16} {'pt':>5} | {'ref asc':>8} {'ref h':>7} | {'our asc':>8} {'our h':>7} | flags")
for fi, fam in enumerate(FONTS):
    for hp in SIZES:
        k = f"F{fi}S{hp}"
        if k not in ref:
            continue
        ra, rh = ref[k]
        oa, oh = ours.get(k, (0, 0))
        flag = ("" if abs(ra - oa) < 1e-6 else "ASC ") + ("" if abs(rh - oh) < 1e-6 else "H")
        print(f"{fam:>16} {hp/2.0:>5.1f} | {ra:>8.2f} {rh:>7.2f} | {oa:>8.2f} {oh:>7.2f} | {flag}")
