#!/usr/bin/env python3
"""The exact natural advance of a candidate line, in both engines.

A justified line's ink box cannot answer "does this word fit": the last glyph's ink is not its
advance, and the question here is decided by a fraction of a point over 482. So the string is set
left-aligned on a page wide enough not to wrap, with a second run right behind it at half the size — a
size change forces its own text record, and that record's pen is the string's advance.
"""
import os, re, subprocess, sys, tempfile, zipfile

REF = "/opt/libreoffice26.2/program/soffice"
CLI = "/home/user/wt-frames/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
OPS = "/home/user/wt-frames/.claude/skills/render-comparison/scripts/pdf-ops.py"

L3_16 = ("road ahead of him before setting out again towards the distant harbour town. "
         "Every carefully measured")
L3_17 = L3_16 + " line"

DOC = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>'
       '{paras}'
       '<w:sectPr><w:pgSz w:w="23812" w:h="16838"/>'
       '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" w:header="0" w:footer="0"/>'
       '</w:sectPr></w:body></w:document>')
PARA = ('<w:p><w:pPr><w:jc w:val="left"/></w:pPr>'
        '<w:r><w:t xml:space="preserve">{t}</w:t></w:r>'
        '<w:r><w:rPr><w:sz w:val="10"/></w:rPr><w:t xml:space="preserve">|</w:t></w:r></w:p>')


def build(path, strings):
    src = "/home/user/wt-frames/dotnet/tests/corpus/features/justify-shrink-2013.docx"
    body = DOC.format(paras="".join(PARA.format(t=s) for s in strings))
    with zipfile.ZipFile(src) as zin, zipfile.ZipFile(path, "w") as zout:
        for item in zin.infolist():
            data = body.encode() if item.filename == "word/document.xml" else zin.read(item.filename)
            zout.writestr(item, data)


def pens(pdf):
    out = subprocess.run([sys.executable, OPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    if not out.strip():
        raise SystemExit(f"{pdf}: pdf-ops produced nothing")
    rows = []
    for line in out.splitlines():
        m = re.match(r"\s*text\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt.*?(\d+) glyphs", line)
        if m:
            rows.append((float(m.group(2)), float(m.group(1)), int(m.group(4))))
    lines = {}
    for y, x, n in rows:
        lines.setdefault(round(y, 1), []).append((x, n))
    return lines


work = tempfile.mkdtemp(dir=os.environ.get("TMPDIR", "/tmp"))
build(f"{work}/probe.docx", [L3_16, L3_17])
prof = tempfile.mkdtemp(dir=work)
subprocess.run([REF, f"-env:UserInstallation=file://{prof}", "--headless", "--norestore",
                "--convert-to", "pdf", "--outdir", f"{work}/ref", f"{work}/probe.docx"],
               capture_output=True)
subprocess.run([CLI, "render", f"{work}/probe.docx", "--outdir", f"{work}/ours"], capture_output=True)

for tag in ("ref", "ours"):
    pdf = f"{work}/{tag}/probe.pdf"
    if not os.path.exists(pdf):
        raise SystemExit(f"{tag}: no PDF written")
    print(f"== {tag}")
    for y in sorted(pens(pdf), reverse=True):
        runs = sorted(pens(pdf)[y])
        if len(runs) < 2:
            continue
        print(f"   baseline {y:8.2f}  start {runs[0][0]:8.3f}  marker {runs[-1][0]:8.3f}"
              f"  advance {runs[-1][0] - runs[0][0]:9.4f} pt")
print("   the measure the justified paragraph has is 9638 twips = 481.9000 pt")
