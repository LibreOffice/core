#!/usr/bin/env python3
"""What sets a pie chart's radius on 26.2.4.2 — read off the reference before the source.

Round 58 measured `003_advanced_excel_pie`'s pie as larger than the reference's and left the
cause open.  This varies exactly one thing at a time in the corpus witness's own chart part,
renders each through the installed binary, and reads the pie's centre and radius back out of
the drawn wedge.

Reading the geometry: the first wedge (#4F81BD) runs from twelve o'clock clockwise through
62.6 degrees, so it lies wholly in the upper-right quadrant and its bounding box's lower-left
corner IS the pie's centre, while its top edge is centre + radius.  That is exact for the
reference, whose arcs arrive polygonised; it is NOT exact for a renderer that emits cubics,
because a bezier's control points sit outside its curve.  Both are reported so the difference
cannot be mistaken for geometry — round 58's "18% larger" is that artefact.

Every variant must produce a rendering AND a wedge, or the case prints FAILED and the run
refuses to summarise: a missing input read as zero reads as a finding.
"""
import os, re, shutil, subprocess, sys, tempfile, zipfile

SRC = "/c/sandbox/workdir/sample-files/sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx"
OUT = "/c/sandbox/workdir/scratch-r59-sheets/pieradius"
PDFOPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"

DLBLS = re.compile(r"<c:dLbls>.*?</c:dLbls>", re.S)
LEGEND = re.compile(r"<c:legend>.*?</c:legend>", re.S)
TITLE = re.compile(r"<c:title>.*?</c:title>\s*<c:autoTitleDeleted val=\"0\"/>", re.S)


def drop_dlbls(x):
    return DLBLS.sub("", x)


def only(x, **flags):
    """Rewrite every show* flag in every c:dLbls / c:dLbl."""
    for k, v in flags.items():
        x = re.sub(r'<c:%s val="[01]"/>' % k, '<c:%s val="%d"/>' % (k, v), x)
    return x


VARIANTS = {
    "00-asis":            lambda x: x,
    "01-nolabels":        drop_dlbls,
    "02-nolegend":        lambda x: LEGEND.sub("", x),
    "03-notitle":         lambda x: TITLE.sub('<c:autoTitleDeleted val="1"/>', x),
    "04-cat-only":        lambda x: only(x, showLegendKey=0, showVal=0, showCatName=1,
                                         showSerName=0, showPercent=0),
    "05-val-only":        lambda x: only(x, showLegendKey=0, showVal=1, showCatName=0,
                                         showSerName=0, showPercent=0),
    "06-nokey":           lambda x: only(x, showLegendKey=0),
    "07-pos-ctr":         lambda x: x.replace('val="bestFit"', 'val="ctr"'),
    "08-pos-inEnd":       lambda x: x.replace('val="bestFit"', 'val="inEnd"'),
    "09-pos-outEnd":      lambda x: x.replace('val="bestFit"', 'val="outEnd"'),
    "10-nolabels-nolegend": lambda x: LEGEND.sub("", drop_dlbls(x)),
    "11-nolabels-notitle": lambda x: TITLE.sub('<c:autoTitleDeleted val="1"/>', drop_dlbls(x)),
    "12-bare":            lambda x: TITLE.sub('<c:autoTitleDeleted val="1"/>',
                                              LEGEND.sub("", drop_dlbls(x))),
    "13-label-8pt":       lambda x: x.replace('sz="1000"', 'sz="800"'),
    "14-label-16pt":      lambda x: x.replace('sz="1000"', 'sz="1600"'),
    "15-cat-only-16pt":   lambda x: only(x.replace('sz="1000"', 'sz="1600"'),
                                         showLegendKey=0, showVal=0, showCatName=1,
                                         showSerName=0, showPercent=0),
}


def build(name, fn):
    dst = os.path.join(OUT, name + ".xlsx")
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "xl/charts/chart1.xml":
                data = fn(data.decode("utf-8")).encode("utf-8")
            zout.writestr(item, data)
    zin.close()
    return dst


def render(path, prof):
    d = os.path.join(OUT, "r-" + os.path.basename(path)[:-5])
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    subprocess.run(["soffice", "-env:UserInstallation=file://" + prof, "--headless",
                    "--convert-to", "pdf", "--outdir", d, path],
                   capture_output=True, timeout=300)
    pdf = os.path.join(d, os.path.basename(path)[:-5] + ".pdf")
    return pdf if os.path.exists(pdf) else None


WEDGE = re.compile(
    r"^fill\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+#4F81BD")


def geometry(pdf):
    """Centre and radius from the first wedge on page 1; None when it is not there."""
    txt = subprocess.run([sys.executable, PDFOPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    best = None
    for line in txt.splitlines():
        m = WEDGE.match(line)
        if not m:
            continue
        x0, y0, x1, y1 = (float(g) for g in m.groups())
        # The legend swatch is the same colour; take the largest such fill.
        if best is None or (x1 - x0) * (y1 - y0) > (best[2] - best[0]) * (best[3] - best[1]):
            best = (x0, y0, x1, y1)
    if best is None:
        return None
    x0, y0, x1, y1 = best
    return (x0, y0, y1 - y0, x1 - x0)      # cx, cy, radius, right-extent


def main():
    os.makedirs(OUT, exist_ok=True)
    prof = os.path.join(OUT, "prof")
    rows, failed = [], []
    for name in sorted(VARIANTS):
        wb = build(name, VARIANTS[name])
        pdf = render(wb, prof)
        g = geometry(pdf) if pdf else None
        if g is None:
            failed.append(name)
            print("  FAILED %s (%s)" % (name, "no wedge" if pdf else "no rendering"))
            continue
        rows.append((name, g))
    if failed:
        print("\nREFUSING TO SUMMARISE — %d of %d variants produced no measurement: %s"
              % (len(failed), len(VARIANTS), ", ".join(failed)), file=sys.stderr)
        sys.exit(2)
    base = rows[0][1][2]
    print("\n%-24s %9s %9s %9s %9s %8s" % ("variant", "cx", "cy", "radius", "diam", "vs 00"))
    for name, (cx, cy, r, w) in rows:
        print("%-24s %9.2f %9.2f %9.2f %9.2f %8.4f" % (name, cx, cy, r, 2 * r, r / base))


if __name__ == "__main__":
    main()
