#!/usr/bin/env python3
"""Re-check: does a stated column width land the next column's text where 26.2.4.2 puts it?

Site: XRay.Office.Spreadsheets/Layout/SheetFonts.cs `DigitWidthCarry` (and the digit-width
model around it), calibrated against LibreOffice 24.2.7.2.

The instrument is indirect on purpose: the x of a glyph in column B *is* the width of
column A, and `pdftotext -bbox` reports it without decoding a content stream.
"""
import os, re, shutil, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from audit_mkwb import workbook

BASE = os.path.dirname(os.path.abspath(__file__))
CLI = "/c/sandbox/workdir/wt-sheets-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")

def render_ref(path):
    d = os.path.join(BASE, "out", "ref"); os.makedirs(d, exist_ok=True)
    stem = os.path.splitext(os.path.basename(path))[0]
    subprocess.run(["soffice", f"-env:UserInstallation=file://{BASE}/prof", "--headless",
                    "--convert-to", "pdf", "--outdir", d, path],
                   capture_output=True, timeout=300, env=ENV)
    return os.path.join(d, stem + ".pdf")

def render_ours(path):
    d = os.path.join(BASE, "out", "ours"); os.makedirs(d, exist_ok=True)
    stem = os.path.splitext(os.path.basename(path))[0]
    subprocess.run([CLI, "render", path, "--format", "pdf", "--outdir", d],
                   capture_output=True, timeout=300, env=ENV)
    return os.path.join(d, stem + ".pdf")

def xof(pdf, token):
    if not os.path.exists(pdf): return None
    out = subprocess.run(["pdftotext", "-bbox", pdf, "-"], capture_output=True).stdout.decode("utf8", "replace")
    for m in re.finditer(r'<word xMin="([\d.]+)" yMin="[\d.]+" xMax="[\d.]+" yMax="[\d.]+">([^<]*)</word>', out):
        if m.group(2) == token:
            return float(m.group(1))
    return None

CASES = []
for font, size in [("Calibri", 11), ("Calibri", 12), ("Liberation Sans", 11),
                   ("Liberation Sans", 12), ("Times New Roman", 11), ("Courier New", 11)]:
    for width in [8.43, 10.0, 12.5, 20.0, 30.0]:
        CASES.append((font, size, width))

print(f"{'font':>18} {'sz':>3} {'width':>6} {'ref x':>9} {'ours x':>9} {'delta pt':>9}")
bad = 0
for font, size, width in CASES:
    name = f"cw_{font.replace(' ','')}_{size}_{width}".replace('.', 'p') + ".xlsx"
    path = workbook(os.path.join(BASE, name), font=font, size=size,
                    cols=[(1, 1, width)], rows=[(1, [("A", "s", "A"), ("B", "s", "MARKER")])])
    r = xof(render_ref(path), "MARKER")
    o = xof(render_ours(path), "MARKER")
    d = (o - r) if (r is not None and o is not None) else None
    if d is None or abs(d) > 0.5: bad += 1
    print(f"{font:>18} {size:>3} {width:>6} {r if r is None else round(r,2):>9} "
          f"{o if o is None else round(o,2):>9} {d if d is None else round(d,3):>9}")
print(f"\ncases: {len(CASES)}  outside 0.5 pt: {bad}")
