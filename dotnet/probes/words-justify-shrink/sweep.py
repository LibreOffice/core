#!/usr/bin/env python3
"""Sweep the text width and read, at each width, whether 26.2.4.2 shrinks the first line.

Two documents per width, identical but for `compatibilityMode` — 12 (shrinking off, so its first
line is the un-shrunk break) and 15 (shrinking on). Reports the first line's word count and mean
word gap for each, so the un-shrunk stretch ratio and the shrunk compression ratio can be read off.
"""
import shutil, subprocess, sys, zipfile, os, tempfile, re, collections

SRC = "/home/user/wt-frames/dotnet/tests/corpus/features/justify-shrink-2013.docx"
REF = "/opt/libreoffice26.2/program/soffice"
SPACE = 463 * 11 / 2048.0   # Carlito's own space advance at 11 pt


def build(path, right_margin, mode):
    with zipfile.ZipFile(SRC) as zin, zipfile.ZipFile(path, "w") as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "word/document.xml":
                s = data.decode()
                s = s.replace('w:right="1134"', f'w:right="{right_margin}"')
                data = s.encode()
            if item.filename == "word/settings.xml":
                data = zin.read(item.filename).decode().replace('w:val="15"', f'w:val="{mode}"').encode()
            zout.writestr(item, data)


def firstline(pdf):
    xml = subprocess.run(["pdftotext", "-bbox", pdf, "-"], capture_output=True, text=True).stdout
    ws = [(round(float(m.group(2)), 1), float(m.group(1)), float(m.group(3)), m.group(5))
          for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', xml)]
    if not ws:
        raise SystemExit(f"{pdf}: no words")
    lines = collections.OrderedDict()
    for y, x0, x1, t in ws:
        lines.setdefault(y, []).append((x0, x1, t))
    first = sorted(next(iter(lines.values())))
    gaps = [first[i + 1][0] - first[i][1] for i in range(len(first) - 1)]
    return len(first), sum(gaps) / len(gaps), first[-1][2]


work = tempfile.mkdtemp(dir=os.environ.get("TMPDIR", "/tmp"))
rows = []
margins = [int(a) for a in sys.argv[1:]] or list(range(1134, 2500, 100))
for rm in margins:
    for mode in (12, 15):
        build(f"{work}/m{rm}_{mode}.docx", rm, mode)
prof = tempfile.mkdtemp(dir=work)
subprocess.run([REF, f"-env:UserInstallation=file://{prof}", "--headless", "--norestore",
                "--convert-to", "pdf", "--outdir", f"{work}/pdf"] + [f"{work}/m{rm}_{m}.docx" for rm in margins for m in (12, 15)],
               capture_output=True)
print(f"{'right':>6} {'measure':>8}  {'mode12 words gap  ratio':>26}   {'mode15 words gap  ratio':>26}  verdict")
for rm in margins:
    got = {}
    for mode in (12, 15):
        p = f"{work}/pdf/m{rm}_{mode}.pdf"
        if not os.path.exists(p):
            raise SystemExit(f"missing {p}")
        got[mode] = firstline(p)
    measure = (11906 - 1134 - rm) / 20.0
    a, b = got[12], got[15]
    ra, rb = (a[1] + 0.034) / SPACE, (b[1] + 0.034) / SPACE
    verdict = "SHRANK" if b[0] > a[0] else ("same" if b[0] == a[0] else "??")
    print(f"{rm:6d} {measure:8.2f}  {a[0]:5d} {a[1]:6.3f} {ra:6.3f}   {b[0]:5d} {b[1]:6.3f} {rb:6.3f}  {verdict}")
