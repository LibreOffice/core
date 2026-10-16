#!/usr/bin/env python3
"""Where exactly does the word `line` stop fitting, in each engine?

One justified paragraph starting at the corpus document's third line, so the decision under test is
its *first* line and nothing upstream can move it. The text width is swept a twip at a time and the
first line's last word read off. The width at which each engine flips is its own answer to "does
this string fit", which is what the residual disagreement is about.
"""
import os, re, subprocess, sys, tempfile, zipfile, collections

REF = "/opt/libreoffice26.2/program/soffice"
CLI = "/home/user/wt-frames/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
TEXT = ("road ahead of him before setting out again towards the distant harbour town. Every carefully "
        "measured line of this paragraph is set justified so that the blanks between its words carry "
        "whatever slack the margin leaves them.")
DOC = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>'
       '<w:p><w:pPr><w:jc w:val="both"/></w:pPr><w:r><w:t xml:space="preserve">{t}</w:t></w:r></w:p>'
       '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
       '<w:pgMar w:top="1134" w:right="{r}" w:bottom="1134" w:left="1134" w:header="0" w:footer="0"/>'
       '</w:sectPr></w:body></w:document>')


def build(path, right):
    src = "/home/user/wt-frames/dotnet/tests/corpus/features/justify-shrink-2013.docx"
    with zipfile.ZipFile(src) as zin, zipfile.ZipFile(path, "w") as zout:
        for item in zin.infolist():
            data = (DOC.format(t=TEXT, r=right).encode()
                    if item.filename == "word/document.xml" else zin.read(item.filename))
            zout.writestr(item, data)


def lastword(pdf):
    xml = subprocess.run(["pdftotext", "-bbox", pdf, "-"], capture_output=True, text=True).stdout
    ws = [(round(float(m.group(2)), 1), float(m.group(1)), m.group(5)) for m in re.finditer(
        r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', xml)]
    if not ws:
        raise SystemExit(f"{pdf}: no words")
    lines = collections.OrderedDict()
    for y, x, t in ws:
        lines.setdefault(y, []).append((x, t))
    first = sorted(next(iter(lines.values())))
    return first[-1][1]


work = tempfile.mkdtemp(dir=os.environ.get("TMPDIR", "/tmp"))
rights = list(range(1128, 1142))
for r in rights:
    build(f"{work}/r{r}.docx", r)
prof = tempfile.mkdtemp(dir=work)
subprocess.run([REF, f"-env:UserInstallation=file://{prof}", "--headless", "--norestore",
                "--convert-to", "pdf", "--outdir", f"{work}/ref"] + [f"{work}/r{r}.docx" for r in rights],
               capture_output=True)
for r in rights:
    done = subprocess.run([CLI, "render", "--outdir", f"{work}/ours", f"{work}/r{r}.docx"],
                          capture_output=True, text=True)
    if done.returncode != 0:
        raise SystemExit(f"render r{r} failed: {done.stderr or done.stdout}")
print(f"{'right':>6} {'measure tw':>11}  {'26.2.4.2':>10}  {'ours':>10}")
for r in rights:
    a, b = f"{work}/ref/r{r}.pdf", f"{work}/ours/r{r}.pdf"
    for f in (a, b):
        if not os.path.exists(f):
            raise SystemExit(f"missing {f}")
    print(f"{r:6d} {11906 - 1134 - r:11d}  {lastword(a):>10}  {lastword(b):>10}")
