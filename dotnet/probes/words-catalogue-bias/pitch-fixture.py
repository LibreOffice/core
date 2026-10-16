#!/usr/bin/env python3
"""Line pitch for a paragraph holding one inline drawing, swept by the drawing's stated height.

The catalogue's rows are paragraphs holding inline `wps:wsp` shapes, and every one of them is a twip
shorter in ours than in the reference. This is the smallest document that shows the same quantity: ten
identical paragraphs, one inline shape each, nothing else on the page, so the pitch between the shapes'
own strokes *is* the line height. `cy` is swept in 1/2-twip steps so a rounding rule shows its boundary.

    pitch-fixture.py <outdir>
"""
import os, re, subprocess, sys, tempfile, zipfile, collections

REF = "/opt/libreoffice26.2/program/soffice"
CLI = "/home/user/wt-frames/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
SRC = "/home/user/wt-frames/dotnet/tests/corpus/features/justify-shrink-2013.docx"
OPS = "/home/user/wt-frames/.claude/skills/render-comparison/scripts/pdf-ops.py"
EMU_PER_TWIP = 635

SHAPE = ('<w:p><w:r><w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0" '
         'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing">'
         '<wp:extent cx="1440000" cy="{cy}"/><wp:effectExtent l="0" t="0" r="0" b="0"/>'
         '<wp:docPr id="{i}" name="s{i}"/>'
         '<a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
         '<a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">'
         '<wps:wsp xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">'
         '<wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x="0" y="0"/>'
         '<a:ext cx="1440000" cy="{cy}"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
         '<a:solidFill><a:srgbClr val="000000"/></a:solidFill><a:ln><a:noFill/></a:ln>'
         '</wps:spPr><wps:bodyPr/></wps:wsp></a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>')
DOC = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>'
       '{body}<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
       '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" w:header="0" w:footer="0"/>'
       '</w:sectPr></w:body></w:document>')


def build(path, cy):
    body = "".join(SHAPE.format(cy=cy, i=i + 1) for i in range(10))
    with zipfile.ZipFile(SRC) as zin, zipfile.ZipFile(path, "w") as zout:
        for item in zin.infolist():
            data = (DOC.format(body=body).encode()
                    if item.filename == "word/document.xml" else zin.read(item.filename))
            zout.writestr(item, data)


def pitch(pdf):
    out = subprocess.run([sys.executable, OPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    if not out.strip():
        raise SystemExit(f"{pdf}: pdf-ops produced nothing")
    ys = sorted({round(float(m.group(2)), 2) for m in re.finditer(
        r"\s*(?:stroke|fill)\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)", out)}, reverse=True)
    if len(ys) < 3:
        raise SystemExit(f"{pdf}: only {len(ys)} drawing records")
    d = [round((ys[i] - ys[i + 1]) * 20, 2) for i in range(len(ys) - 1)]
    return ys, d


work = sys.argv[1] if len(sys.argv) > 1 else tempfile.mkdtemp(dir=os.environ.get("TMPDIR", "/tmp"))
os.makedirs(work, exist_ok=True)
cys = [EMU_PER_TWIP * n for n in range(560, 572)]
for cy in cys:
    build(f"{work}/cy{cy}.docx", cy)
prof = tempfile.mkdtemp(dir=work)
subprocess.run([REF, f"-env:UserInstallation=file://{prof}", "--headless", "--norestore",
                "--convert-to", "pdf", "--outdir", f"{work}/ref"] + [f"{work}/cy{cy}.docx" for cy in cys],
               capture_output=True)
for cy in cys:
    done = subprocess.run([CLI, "render", "--quiet", "--outdir", f"{work}/ours", f"{work}/cy{cy}.docx"],
                          capture_output=True, text=True)
    if done.returncode:
        raise SystemExit(done.stderr or done.stdout)
print(f"{'cy, twips':>10} {'26.2.4.2 pitch, tw':>20} {'ours, tw':>12} {'difference':>11}")
for cy in cys:
    a, b = f"{work}/ref/cy{cy}.pdf", f"{work}/ours/cy{cy}.pdf"
    for f in (a, b):
        if not os.path.exists(f):
            raise SystemExit(f"missing {f}")
    _, pr = pitch(a)
    _, po = pitch(b)
    print(f"{cy // EMU_PER_TWIP:10d} {pr[0]:20.2f} {po[0]:12.2f} {pr[0] - po[0]:11.2f}")
