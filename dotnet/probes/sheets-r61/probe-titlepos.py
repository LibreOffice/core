#!/usr/bin/env python3
"""Where 26.2.4.2 puts a chart's MAIN title, as a size series, against where we put ours.

Round 60 measured one number on two documents: our title's first baseline is 9.57 pt higher
than the reference's, at 18 pt.  This probe turns that into a law by varying the one thing the
candidate law depends on — the title's own font size.

The candidate, read off `ChartView.cxx:1058-1069` and `ShapeFactory.cxx:2279-2299` before any
of our own source was opened:

    title shape top = frame.Y + int(frameHeight_mm100 * 0.02) + 135          (MAIN_TITLE only)
    text top        = shape top + round(fontHeight_mm100 * 0.30)            (TextUpperDistance)

Ours puts the text top at `frame.Y + frameHeight * 0.02` and nothing else, so if the two sides'
ascents agree — which round 60 measured to 0.04-0.07 pt — then

    y_ours - y_ref  ==  135/100 mm  +  round(size_mm100 * 0.30)/100 mm

with no free parameter at all.  A constant residual is an ascent error; a residual that tracks
the size is a wrong coefficient.

Refuses to summarise unless every case produced exactly one title run on both sides.
"""
import os
import re
import shutil
import subprocess
import sys
import zipfile

SRC = "/c/sandbox/workdir/sample-files/sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx"
OUT = "/c/sandbox/workdir/scratch-r61-sheets/titlepos"
ROOT = "/c/sandbox/workdir/wt-sheets-r50"
PDFOPS = ROOT + "/.claude/skills/render-comparison/scripts/pdf-ops.py"
CLI = ROOT + "/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"

SIZES = [600, 800, 1000, 1200, 1400, 1800, 2200, 2800, 3600]
BOLDS = [1, 0]

TEXT = re.compile(r"^text\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt")
TITLE_RUN = re.compile(r'<a:r><a:rPr sz="1800" b="1".*?</a:r>', re.S)
DLBLS = re.compile(r"<c:dLbls>.*?</c:dLbls>", re.S)


def build(name, size, bold):
    dst = os.path.join(OUT, name + ".xlsx")
    run = ('<a:r><a:rPr sz="%d" b="%d" u="none" strike="noStrike">'
           '<a:solidFill><a:srgbClr val="000000"/></a:solidFill><a:uFillTx/>'
           '<a:latin typeface="Calibri"/></a:rPr><a:t>Wg</a:t></a:r>' % (size, bold))
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "xl/charts/chart1.xml":
                x = data.decode("utf-8")
                x = DLBLS.sub("", x)                       # nothing else at any size
                x = TITLE_RUN.sub(run, x)
                data = x.encode("utf-8")
            zout.writestr(item, data)
    zin.close()
    return dst


def render_ref(name, wb):
    d = os.path.join(OUT, "ref-" + name)
    out = os.path.join(d, name + ".pdf")
    if os.path.exists(out):
        return out
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    subprocess.run(["soffice", "-env:UserInstallation=file://" + os.path.join(OUT, "prof"),
                    "--headless", "--convert-to", "pdf", "--outdir", d, wb],
                   capture_output=True, timeout=600)
    return out


def render_ours(name, wb):
    d = os.path.join(OUT, "our-" + name)
    out = os.path.join(d, name + ".pdf")
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    subprocess.run([CLI, "render", wb, "--format", "pdf", "--outdir", d],
                   capture_output=True, timeout=600)
    return out


def title_run(pdf, size_pt):
    """The one run on page 1 drawn at the title's size inside the chart frame."""
    if not os.path.exists(pdf):
        return None
    txt = subprocess.run([sys.executable, PDFOPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    hits = []
    for line in txt.splitlines():
        m = TEXT.match(line)
        if not m:
            continue
        x, y, sz = float(m.group(1)), float(m.group(2)), float(m.group(3))
        if 200 < x < 640 and 330 < y < 640 and abs(sz - size_pt) < 0.25:
            hits.append((x, y, sz))
    return hits


def main():
    os.makedirs(OUT, exist_ok=True)
    rows, bad = [], []
    for bold in BOLDS:
        for size in SIZES:
            name = "t%d_b%d" % (size, bold)
            wb = build(name, size, bold)
            pts = size / 100.0
            ours = title_run(render_ours(name, wb), pts)
            ref = title_run(render_ref(name, wb), pts)
            if not ours or not ref or len(ours) != 1 or len(ref) != 1:
                bad.append((name, ours, ref))
                continue
            rows.append((size, bold, ours[0][1], ref[0][1], ours[0][0], ref[0][0]))

    if bad:
        print("REFUSING TO SUMMARISE — %d cases did not give one title run per side:" % len(bad))
        for b in bad:
            print("  ", b)
        return 2

    mm100 = 2540.0 / 72.0
    print("%-6s %-4s %9s %9s %8s %9s %8s %8s" %
          ("size", "bold", "y_ours", "y_ref", "D", "predict", "resid", "dx"))
    for size, bold, yo, yr, xo, xr in rows:
        upper = round(size / 100.0 * mm100 * 0.30)
        predict = (135 + upper) / mm100
        d = yo - yr
        print("%-6.1f %-4d %9.2f %9.2f %8.3f %9.3f %8.3f %8.2f" %
              (size / 100.0, bold, yo, yr, d, predict, d - predict, xo - xr))
    return 0


if __name__ == "__main__":
    sys.exit(main())
