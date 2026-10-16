#!/usr/bin/env python3
"""A chart text shape's line pitch and ascent on 26.2.4.2, as a size series on three faces.

Round 59 left two points and no law.  The first cut of this probe (`probe-chartvmetrics.py`)
tried to read the pitch off the pie's own data labels and could not: above about twelve point the
five labels overlap each other at `ctr` and the runs cannot be grouped into blocks.  This one
separates the two measurements onto two instruments that each have a clean witness.

  A. **pitch** — the chart *title*, rewritten as N single-glyph-pair lines joined by `<a:br/>`,
     with the data labels deleted so nothing else is drawn at the title's size.  A title is made
     by the same `ShapeFactory::createText` as a label, and the 10 pt reading agrees with the
     label pitch measured on the unmodified witness (11.22), which is the control.

  B. **ascent** — the data labels, reduced to `showVal` only so each is one short run that never
     wraps and never collides, and pinned at `dLblPos="ctr"`.  Round 59 measured that `ctr` does
     not shrink the diagram, so each label's block centre `C` is the same at every font size:
     `y1(s) = C + H(s)/2 - A(s)`, and the slope of `y1` against `s` is `h/2 - a`.  `C` cancels.

Nothing is fitted to our own renderer; every number comes out of the installed binary.

Refuses to summarise unless every case produced the geometry it was asked for.
"""
import os
import re
import shutil
import subprocess
import sys
import zipfile

SRC = "/c/sandbox/workdir/sample-files/sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx"
OUT = "/c/sandbox/workdir/scratch-r60-sheets/vmetrics2"
PDFOPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"

FACES = [("Calibri", "Carlito"), ("Arial", "Liberation Sans"),
         ("Times New Roman", "Liberation Serif")]
SIZES = [600, 800, 1000, 1100, 1200, 1400, 1600, 1800, 2000, 2400, 2800, 3200, 4000]
# Above about twenty point a CENTER label wraps and the one-line reading is no longer one line;
# the ascent series therefore runs on the sizes that stay on one line, which is stated rather
# than silently dropped.
SIZES_B = [600, 800, 1000, 1100, 1200, 1400, 1600, 1800, 2000, 2400]

TEXT = re.compile(r"^text\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt")
DLBLS = re.compile(r"<c:dLbls>.*?</c:dLbls>", re.S)
TITLE_P = re.compile(r"(<c:rich><a:bodyPr[^>]*/><a:lstStyle/>)<a:p>.*?</a:p>", re.S)


def title_paragraph(face, size, lines):
    runs = []
    for i in range(lines):
        if i:
            runs.append("<a:br/>")
        runs.append('<a:r><a:rPr sz="%d" b="0" u="none" strike="noStrike">'
                    '<a:solidFill><a:srgbClr val="000000"/></a:solidFill>'
                    '<a:latin typeface="%s"/></a:rPr><a:t>Mg</a:t></a:r>' % (size, face))
    return ('<a:p><a:pPr><a:defRPr sz="%d" b="0"><a:latin typeface="%s"/></a:defRPr></a:pPr>%s</a:p>'
            % (size, face, "".join(runs)))


def build(name, mutate):
    dst = os.path.join(OUT, name + ".xlsx")
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "xl/charts/chart1.xml":
                data = mutate(data.decode("utf-8")).encode("utf-8")
            zout.writestr(item, data)
    zin.close()
    return dst


def pitch_case(face, size, lines):
    def mutate(x):
        x = DLBLS.sub("", x)
        x = TITLE_P.sub(lambda m: m.group(1) + title_paragraph(face, size, lines), x)
        return x
    return mutate


def ascent_case(face, size):
    def mutate(x):
        x = x.replace('<c:dLblPos val="bestFit"/>', '<c:dLblPos val="ctr"/>')
        x = x.replace('<c:showLegendKey val="1"/>', '<c:showLegendKey val="0"/>')
        x = x.replace('<c:showCatName val="1"/>', '<c:showCatName val="0"/>')
        x = x.replace('<c:showSerName val="1"/>', '<c:showSerName val="0"/>')
        x = x.replace('<c:showPercent val="1"/>', '<c:showPercent val="0"/>')
        x = x.replace('<a:defRPr sz="1000"', '<a:defRPr sz="%d"' % size)
        x = re.sub(
            r'(<a:defRPr sz="%d"(?:(?!</a:defRPr>).)*?<a:latin typeface=")[^"]+(")' % size,
            r"\g<1>%s\g<2>" % face, x, flags=re.S)
        return x
    return mutate


def render(name, wb):
    d = os.path.join(OUT, "r-" + name)
    cached = os.path.join(d, name + ".pdf")
    if os.path.exists(cached):
        return cached
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    subprocess.run(
        ["soffice", "-env:UserInstallation=file://" + os.path.join(OUT, "prof"),
         "--headless", "--convert-to", "pdf", "--outdir", d, wb],
        capture_output=True, timeout=600)
    return os.path.join(d, name + ".pdf")


def chart_runs(pdf, size_pt, lo=341.0, hi=625.0):
    """Runs drawn inside the chart frame at the stated size — the title, or the labels."""
    txt = subprocess.run([sys.executable, PDFOPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    out = []
    for line in txt.splitlines():
        m = TEXT.match(line)
        if not m:
            continue
        x, y, sz = float(m.group(1)), float(m.group(2)), float(m.group(3))
        if not (200 < x < 600 and lo < y < hi):
            continue
        if abs(sz - size_pt) > max(0.3, size_pt * 0.03):
            continue
        out.append((x, y, sz))
    return out


def main():
    os.makedirs(OUT, exist_ok=True)
    missing = []
    pitch = {}
    firstline = {}
    ascent = {}

    for stated, resolved in FACES:
        for size in SIZES:
            s = size / 100.0
            for lines in (1, 3):
                name = "t-%s-%d-%d" % (stated.replace(" ", ""), size, lines)
                pdf = render(name, build(name, pitch_case(stated, size, lines)))
                rows = chart_runs(pdf, s) if os.path.exists(pdf) else []
                rows.sort(key=lambda r: -r[1])
                if len(rows) != lines:
                    missing.append("%s: %d runs, wanted %d" % (name, len(rows), lines))
                    continue
                if lines == 1:
                    firstline[(stated, size, 1)] = rows[0][1]
                else:
                    firstline[(stated, size, 3)] = rows[0][1]
                    gaps = [rows[i][1] - rows[i + 1][1] for i in range(len(rows) - 1)]
                    pitch[(stated, size)] = (sorted(gaps), rows[0][2])

            if size not in SIZES_B:
                continue
            name = "a-%s-%d" % (stated.replace(" ", ""), size)
            pdf = render(name, build(name, ascent_case(stated, size)))
            # the title is drawn at 18 pt inside the same frame; keep the plot area only
            rows = chart_runs(pdf, s, hi=570.0) if os.path.exists(pdf) else []
            if len(rows) != 5:
                missing.append("%s: %d label runs, wanted 5" % (name, len(rows)))
                continue
            ascent[(stated, size)] = (sorted(r[1] for r in rows), rows[0][2])

    if missing:
        print("REFUSING TO SUMMARISE — cases with no measurable geometry:", file=sys.stderr)
        for m in missing:
            print("   ", m, file=sys.stderr)
        sys.exit(2)

    print("A. line pitch inside one chart text shape")
    print("%-16s %7s %8s %9s %9s %9s" %
          ("face", "stated", "drawn", "pitch", "pitch/em", "top-fixed"))
    for stated, resolved in FACES:
        for size in SIZES:
            gaps, drawn = pitch[(stated, size)]
            p = gaps[len(gaps) // 2]
            shift = firstline[(stated, size, 1)] - firstline[(stated, size, 3)]
            print("%-16s %7.2f %8.2f %9.3f %9.4f %9.3f" %
                  (resolved, size / 100.0, drawn, p, p / drawn, shift))

    print("\nB. first baseline of a one-line CENTER label, against the stated size")
    for stated, resolved in FACES:
        xs, yss = [], []
        for size in SIZES_B:
            ys, drawn = ascent[(stated, size)]
            xs.append(drawn)
            yss.append(ys)
        n = min(len(y) for y in yss)
        slopes = []
        for k in range(n):
            ys = [y[k] for y in yss]
            mx = sum(xs) / len(xs)
            my = sum(ys) / len(ys)
            den = sum((x - mx) ** 2 for x in xs)
            slopes.append(sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / den)
        slopes.sort()
        slope = slopes[len(slopes) // 2]
        print("  %-16s dy1/ds = %+7.4f em   per-anchor: %s"
              % (resolved, slope, " ".join("%+.4f" % v for v in slopes)))
        for size in SIZES:
            gaps, drawn = pitch[(stated, size)]
            h = gaps[len(gaps) // 2] / drawn
            print("      at %5.2f pt: h = %.4f  =>  a = h/2 - slope = %.4f"
                  % (size / 100.0, h, h / 2 - slope))


if __name__ == "__main__":
    main()
