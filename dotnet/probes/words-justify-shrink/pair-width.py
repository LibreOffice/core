#!/usr/bin/env python3
"""The corpus pair at three text widths: does the residual survive a one-twip move?"""
import os, re, subprocess, sys, tempfile, zipfile, collections

REF = "/opt/libreoffice26.2/program/soffice"
CLI = "/home/user/wt-frames/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
SRC = "/home/user/wt-frames/dotnet/tests/corpus/features/justify-shrink-{m}.docx"


def build(path, mode, right):
    with zipfile.ZipFile(SRC.format(m=mode)) as zin, zipfile.ZipFile(path, "w") as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "word/document.xml":
                data = data.decode().replace('w:right="1134"', f'w:right="{right}"').encode()
            zout.writestr(item, data)


def lines(pdf):
    xml = subprocess.run(["pdftotext", "-bbox", pdf, "-"], capture_output=True, text=True).stdout
    ws = [(round(float(m.group(2)), 1), float(m.group(1)), m.group(5)) for m in re.finditer(
        r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', xml)]
    if not ws:
        raise SystemExit(f"{pdf}: no words")
    out = collections.OrderedDict()
    for y, x, t in ws:
        out.setdefault(y, []).append((x, t))
    return [sorted(v)[-1][1] for v in out.values()]


work = tempfile.mkdtemp(dir=os.environ.get("TMPDIR", "/tmp"))
rights = [int(a) for a in sys.argv[1:]] or [1133, 1134, 1135]
files = []
for r in rights:
    for m in ("2013", "2007"):
        p = f"{work}/{m}_{r}.docx"
        build(p, m, r)
        files.append(p)
prof = tempfile.mkdtemp(dir=work)
subprocess.run([REF, f"-env:UserInstallation=file://{prof}", "--headless", "--norestore",
                "--convert-to", "pdf", "--outdir", f"{work}/ref"] + files, capture_output=True)
for f in files:
    done = subprocess.run([CLI, "render", "--outdir", f"{work}/ours", f], capture_output=True, text=True)
    if done.returncode:
        raise SystemExit(done.stderr or done.stdout)
for r in rights:
    for m in ("2013", "2007"):
        a, b = f"{work}/ref/{m}_{r}.pdf", f"{work}/ours/{m}_{r}.pdf"
        for f in (a, b):
            if not os.path.exists(f):
                raise SystemExit(f"missing {f}")
        ra, rb = lines(a), lines(b)
        verdict = "MATCH" if ra == rb else "differ"
        print(f"right={r} mode {m}: {verdict}")
        print(f"    ref  {len(ra)} lines end {ra}")
        print(f"    ours {len(rb)} lines end {rb}")
