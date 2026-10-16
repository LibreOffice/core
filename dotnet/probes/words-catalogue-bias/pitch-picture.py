#!/usr/bin/env python3
"""The same pitch sweep as `pitch-fixture.py`, with an inline *picture* instead of a shape.

Which of the two carries the extra twip decides the reach of any fix, so the two fixtures differ in
exactly one thing: what the `a:graphicData` holds.
"""
import os, re, struct, subprocess, sys, tempfile, zlib, zipfile

REF = "/opt/libreoffice26.2/program/soffice"
CLI = "/home/user/wt-frames/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
SRC = "/home/user/wt-frames/dotnet/tests/corpus/features/justify-shrink-2013.docx"
OPS = "/home/user/wt-frames/.claude/skills/render-comparison/scripts/pdf-ops.py"
EMU_PER_TWIP = 635


def png(w=8, h=8):
    raw = b"".join(b"\x00" + bytes([0, 0, 0]) * w for _ in range(h))
    def chunk(tag, data):
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))
    return (b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw))
            + chunk(b"IEND", b""))


PIC = ('<w:p><w:r><w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0" '
       'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing">'
       '<wp:extent cx="1440000" cy="{cy}"/><wp:effectExtent l="0" t="0" r="0" b="0"/>'
       '<wp:docPr id="{i}" name="p{i}"/>'
       '<a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
       '<a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">'
       '<pic:pic xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture">'
       '<pic:nvPicPr><pic:cNvPr id="{i}" name="p{i}"/><pic:cNvPicPr/></pic:nvPicPr>'
       '<pic:blipFill><a:blip xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" r:embed="rIdImg"/>'
       '<a:stretch><a:fillRect/></a:stretch></pic:blipFill>'
       '<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="1440000" cy="{cy}"/></a:xfrm>'
       '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>'
       '</a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>')
DOC = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>'
       '{body}<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
       '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" w:header="0" w:footer="0"/>'
       '</w:sectPr></w:body></w:document>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rIdImg" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image"'
        ' Target="media/i.png"/></Relationships>')


def build(path, cy):
    body = "".join(PIC.format(cy=cy, i=i + 1) for i in range(10))
    with zipfile.ZipFile(SRC) as zin, zipfile.ZipFile(path, "w") as zout:
        for item in zin.infolist():
            if item.filename == "word/document.xml":
                zout.writestr(item, DOC.format(body=body).encode())
            elif item.filename == "word/_rels/document.xml.rels":
                zout.writestr(item, RELS.encode())
            elif item.filename == "[Content_Types].xml":
                s = zin.read(item.filename).decode()
                s = s.replace("</Types>", '<Default Extension="png" ContentType="image/png"/></Types>')
                zout.writestr(item, s.encode())
            else:
                zout.writestr(item, zin.read(item.filename))
        zout.writestr("word/media/i.png", png())


def pitch(pdf):
    out = subprocess.run([sys.executable, OPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    if not out.strip():
        raise SystemExit(f"{pdf}: pdf-ops produced nothing")
    ys = sorted({round(float(m.group(2)), 2) for m in re.finditer(
        r"\s*image\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)", out)}, reverse=True)
    if len(ys) < 3:
        raise SystemExit(f"{pdf}: only {len(ys)} images")
    return [round((ys[i] - ys[i + 1]) * 20, 2) for i in range(len(ys) - 1)]


work = sys.argv[1] if len(sys.argv) > 1 else tempfile.mkdtemp(dir=os.environ.get("TMPDIR", "/tmp"))
os.makedirs(work, exist_ok=True)
cys = [EMU_PER_TWIP * n for n in range(560, 566)]
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
    pr, po = pitch(a)[0], pitch(b)[0]
    print(f"{cy // EMU_PER_TWIP:10d} {pr:20.2f} {po:12.2f} {pr - po:11.2f}")
