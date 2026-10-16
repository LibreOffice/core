#!/usr/bin/env python3
"""What line height does the installed LibreOffice give Liberation Serif at each size?

Reads consecutive baselines out of the reference PDF's own text matrices, so the answer is
measured rather than derived from the font's hhea table. The 12 pt case is the control: the
tree already reproduces it to 0.00 pt, so a size where the two part company is a defect and
not a difference of convention.
"""
import os, subprocess, sys, re, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx import write

OPS = "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
OUT = sys.argv[1] if len(sys.argv) > 1 else tempfile.mkdtemp()
os.makedirs(OUT, exist_ok=True)

SIZES = [12, 14, 16, 18, 20, 22, 24, 28]   # half-points: 6 pt .. 14 pt

body = []
for hp in SIZES:
    for i in range(3):
        body.append(
            f'<w:p><w:pPr><w:rPr><w:sz w:val="{hp}"/></w:rPr></w:pPr>'
            f'<w:r><w:rPr><w:sz w:val="{hp}"/></w:rPr><w:t>S{hp}L{i} Hxg</w:t></w:r></w:p>')
    body.append('<w:p/>')

src = write(os.path.join(OUT, "lineheight.docx"), "".join(body))
subprocess.run(["soffice", "-env:UserInstallation=file://" + OUT + "/prof",
                "--headless", "--convert-to", "pdf", "--outdir", OUT, src],
               capture_output=True)
pdf = os.path.join(OUT, "lineheight.pdf")

out = subprocess.run(["python3", OPS, "dump", pdf, "--page", "1", "--only", "text"],
                     capture_output=True, text=True).stdout
rows = []
for l in out.splitlines():
    m = re.search(r'\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt', l)
    if m:
        rows.append((float(m.group(2)), float(m.group(3))))
rows.sort(key=lambda r: -r[0])

print(f"{'pt':>6} {'baseline gap':>14}  {'gap/size':>9}")
by = {}
for y, sz in rows:
    by.setdefault(sz, []).append(y)
for sz in sorted(by):
    ys = sorted(by[sz], reverse=True)
    gaps = [round(ys[i] - ys[i + 1], 4) for i in range(len(ys) - 1)]
    g = gaps[0] if gaps else 0
    print(f"{sz:>6.1f} {g:>14.2f}  {g / sz if sz else 0:>9.4f}   all={gaps}")
