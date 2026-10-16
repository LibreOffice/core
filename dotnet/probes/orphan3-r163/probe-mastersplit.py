#!/usr/bin/env python3
"""Does the MASTER part of a split table row pay the space-after of the cells that finished?

One row, two cells. Cell A ("Tag") holds one line and finishes above the cut.
Cell B holds a string that wraps to two lines, so the row splits. Every cell paragraph
carries w:spacing w:before=40 (2 pt) and a swept w:after.

If the master part's rule-to-rule height grows with `after`, the finished cell's
trailing spacing is charged there; if it does not, it is not.
"""
import os, re, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx2 import write

OPS = "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
SOFFICE = "/opt/libreoffice26.2/program/soffice"
CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli"
OUT = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else "fixture")
os.makedirs(OUT, exist_ok=True)

BORDERS = ('<w:tblBorders>' + "".join(
    f'<w:{e} w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
    for e in ("top", "left", "bottom", "right", "insideH", "insideV")) + '</w:tblBorders>')

def para(before, after, text):
    return (f'<w:p><w:pPr><w:spacing w:before="{before}" w:after="{after}"'
            f' w:line="233" w:lineRule="auto"/></w:pPr>'
            f'<w:r><w:t xml:space="preserve">{text}</w:t></w:r></w:p>')

def doc(after, filler):
    pre = "".join(f'<w:p><w:pPr><w:spacing w:before="0" w:after="0"/></w:pPr>'
                  f'<w:r><w:t>Filler {i}</w:t></w:r></w:p>' for i in range(filler))
    def tc(w, p):
        return f'<w:tc><w:tcPr><w:tcW w:w="{w}" w:type="dxa"/></w:tcPr>{p}</w:tc>'
    rows = ''
    # a couple of ordinary anchor rows so the unsplit row height is measurable too
    for i in range(2):
        rows += ('<w:tr>' + tc(1460, para(40, after, f"Anchor {i}"))
                 + tc(1109, para(40, after, "short")) + '</w:tr>')
    # the target row: cell A one line, cell B wraps to two
    rows += ('<w:tr>' + tc(1460, para(40, after, "Tag"))
             + tc(1109, para(40, after, "05/2010")) + '</w:tr>')
    tbl = ('<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/>' + BORDERS + '</w:tblPr>'
           '<w:tblGrid><w:gridCol w:w="1460"/><w:gridCol w:w="1109"/></w:tblGrid>'
           + rows + '</w:tbl><w:p/>')
    return pre + tbl

def rules(pdf, page):
    out = subprocess.run(["python3", OPS, "dump", pdf, "--page", str(page)],
                         capture_output=True, text=True).stdout
    ys, texts = [], []
    for l in out.splitlines():
        m = re.match(r'stroke\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)', l)
        if m:
            x1, y1, x2, y2 = map(float, m.groups())
            if abs(y1 - y2) < 0.01 and x2 - x1 > 100:
                ys.append(round(y1, 2))
        m = re.match(r'text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)', l)
        if m: texts.append(round(float(m.group(2)), 2))
    return sorted(set(ys), reverse=True), sorted(set(texts), reverse=True)

def render(src, who, tag):
    d = os.path.join(OUT, who, tag); os.makedirs(d, exist_ok=True)
    if who == "ours":
        subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", d], capture_output=True)
    else:
        subprocess.run([SOFFICE, "-env:UserInstallation=file://" + OUT + "/prof-" + who,
                        "--headless", "--norestore", "--convert-to", "pdf", "--outdir", d, src],
                       capture_output=True)
    stem = os.path.splitext(os.path.basename(src))[0]
    p = os.path.join(d, stem + ".pdf")
    return p if os.path.exists(p) else None

print(f"{'after':>6} {'filler':>6} {'who':>5} {'pg1 rules(last 3)':>26} {'master h':>9} {'p2 rules(first 3)':>26} {'follow h':>9}")
for after in (0, 40, 240):
    for filler in range(38, 50):
        tag = f"a{after}-f{filler}"
        src = os.path.join(OUT, tag + ".docx")
        write(src, doc(after, filler))
        row = {}
        for who in ("ref", "ours"):
            pdf = render(src, who, tag)
            if not pdf: row[who] = None; continue
            n = int(subprocess.run(["pdfinfo", pdf], capture_output=True, text=True)
                    .stdout.split("Pages:")[1].split()[0])
            if n < 2: row[who] = ("1page",); continue
            r1, t1 = rules(pdf, 1); r2, t2 = rules(pdf, 2)
            # is the last row split?  page 2 must carry table rules and text
            row[who] = (r1[-3:], round(r1[-2] - r1[-1], 2) if len(r1) >= 2 else None,
                        r2[:3], round(r2[0] - r2[1], 2) if len(r2) >= 2 else None,
                        len(t1), len(t2))
        if row.get("ref") and row["ref"][0] != "1page" and row.get("ours"):
            for who in ("ref", "ours"):
                v = row[who]
                if v == ("1page",): print(f"{after:>6} {filler:>6} {who:>5}  single page"); continue
                print(f"{after:>6} {filler:>6} {who:>5} {str(v[0]):>26} {str(v[1]):>9} {str(v[2]):>26} {str(v[3]):>9}  txt {v[4]}/{v[5]}")
            print()
