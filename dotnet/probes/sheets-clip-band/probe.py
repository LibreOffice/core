#!/usr/bin/env python3
"""Where does LibreOffice clip a header or footer band, and do we clip in the same place?

`ScPrintFunc::PrintHF` sets one clip region before it draws a band's three areas
(`sc/source/ui/view/printfun.cxx:1870`) and every area goes through it. The rectangle is
readable straight out of the reference's own content stream as a `re W* n`, so this reads
both sides' rectangles rather than inferring them from ink.

    python3 probe.py <out-dir> [document ...]

Prints one row per band per side: the rectangle in top-down page points.
"""
import glob, os, re, subprocess, sys, zlib

OUT = sys.argv[1]
DOCS = sys.argv[2:] or [
    "/home/user/sample-files/sheets/done-011/xlsx/FY2023-AIP-grants.xlsx",
]
CLI = os.environ.get("XRAY_OFFICE_CLI")
LO = {"24.2": os.environ.get("LO24", "soffice"),
      "26.2": os.environ.get("LO26", "/opt/libreoffice26.2/program/soffice")}
PAGE = int(os.environ.get("PAGE", "3"))
os.makedirs(OUT, exist_ok=True)


def render(who, src):
    d = os.path.join(OUT, who.replace(".", ""), os.path.basename(src))
    os.makedirs(d, exist_ok=True)
    got = glob.glob(os.path.join(d, "*.pdf"))
    if got:
        return got[0]
    if who == "ours":
        subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", d],
                       capture_output=True, timeout=1800)
    else:
        subprocess.run([LO[who], f"-env:UserInstallation=file://{d}/prof", "--headless",
                        "--convert-to", "pdf", "--outdir", d, src],
                       capture_output=True, timeout=1800)
    got = glob.glob(os.path.join(d, "*.pdf"))
    return got[0] if got else None


def clips(pdf, page):
    """Every `x y w h re W* n` on one page, as (top, height, left, width) top-down."""
    data = open(pdf, "rb").read()
    height = 612.0
    info = subprocess.run(["pdfinfo", pdf], capture_output=True, text=True).stdout
    m = re.search(r"Page size:\s+([\d.]+) x ([\d.]+)", info)
    if m:
        height = float(m.group(2))
    seen = 0
    for m in re.finditer(rb"stream\r?\n", data):
        start = m.end()
        end = data.find(b"endstream", start)
        try:
            text = zlib.decompress(data[start:end]).decode("latin1")
        except Exception:
            continue
        if "Tj" not in text and "TJ" not in text:
            continue
        seen += 1
        if seen != page:
            continue
        out = []
        for r in re.finditer(r"([-\d.]+) ([-\d.]+) ([-\d.]+) ([-\d.]+) re\s*\n?W\*? n", text):
            x, y, w, h = (float(v) for v in r.groups())
            out.append((round(height - y - h, 3), round(h, 3), round(x, 3), round(w, 3)))
        return out
    return []


who = ["24.2", "26.2"] + (["ours"] if CLI else [])
print(f"{'side':>6} {'top':>9} {'height':>8} {'left':>8} {'width':>9}  document")
for src in DOCS:
    for w in who:
        pdf = render(w, src)
        if not pdf:
            print(f"{w:>6}   (no render)  {os.path.basename(src)}")
            continue
        # A band spans the page between its margins and is short; a cell clip is neither.
        got = [c for c in clips(pdf, PAGE) if c[1] < 100 and c[3] > 400]
        if not got:
            print(f"{w:>6} {'(none)':>9}{'':>28}  {os.path.basename(src)}")
        for top, h, left, width in got:
            print(f"{w:>6} {top:>9.3f} {h:>8.3f} {left:>8.3f} {width:>9.3f}  "
                  f"{os.path.basename(src)}")
