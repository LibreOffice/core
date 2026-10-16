#!/usr/bin/env python3
"""The law behind a chart label's line height and ascent, read off 26.2.4.2 itself.

Round 59 left two points and no law: the reference stacks Carlito's chart-label lines at
**1.1219 em at 10.01 pt** and **1.2241 em at 15.89 pt**, where our `ChartLineHeightAt` answers
the face's own `ascent + descent + lineGap` — 1.2207 for Carlito at every size.  Sub-linear, so
no scaled sum of the face's metrics is right at both.  Two points do not separate "a different
table" from "integer rounding onto a device grid", and only a size *series* does.

METHOD — everything is read out of the reference's own renderings, nothing is fitted to ours
──────────────────────────────────────────────────────────────────────────────────────────────
One-variable rewrites of `003_advanced_excel_pie.xlsx`'s own chart part:

  * `c:dLblPos` is forced to `ctr` on every label.  Round 59 measured that `ctr` does **not**
    shrink the diagram (radius 110.44, the same as with no labels at all), so the pie's centre
    and radius — and therefore each label block's anchor point `C` — are identical across every
    case in the series.  That is what makes the two readings below independent of the geometry.
  * `c:separator` is either `; ` (one line per label) or a newline (four lines per label).

Two quantities come out, per face and per stated size:

  H  the line height, as the median baseline-to-baseline distance inside a four-line label.
  a  the *ascent* per em, from the size series rather than from any one rendering.  A CENTER
     label's block centre `C` does not depend on the font size, and its first baseline is
     `y1(s) = C + H(s)/2 - A(s)`.  With `H(s) = h·s` and `A(s) = a·s` the slope of `y1` against
     `s` is `h/2 - a`, so `a = h/2 - slope`.  `C` never has to be known.

The one-line and four-line renderings give H twice by different arithmetic — `y1(4) - y1(1)`
is `1.5·H` — and the probe prints both so a disagreement is visible rather than averaged away.

Arial (which resolves to Liberation Sans here) runs the same series as a control: our answer for
that face is believed right, so a face whose measured `h` comes back at our 1.1499 is evidence the
instrument is sound, and one that does not is evidence the defect is not Carlito's alone.

Refuses to summarise unless every case produced both a four-line spacing and a one-line baseline.
"""
import os
import re
import shutil
import subprocess
import sys
import zipfile

SRC = "/c/sandbox/workdir/sample-files/sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx"
OUT = "/c/sandbox/workdir/scratch-r60-sheets/vmetrics"
PDFOPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"

FACES = [("Calibri", "Carlito"), ("Arial", "Liberation Sans")]
SIZES = [800, 1000, 1200, 1400, 1600, 1800, 2000, 2400]

TEXT = re.compile(
    r"^text\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt\S*\s+(\S+)")


def build(name, face, size, lines):
    """Rewrite the witness's chart part: one face, one size, ctr labels, 1 or 4 lines."""
    dst = os.path.join(OUT, name + ".xlsx")
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "xl/charts/chart1.xml":
                x = data.decode("utf-8")
                x = x.replace('<c:dLblPos val="bestFit"/>', '<c:dLblPos val="ctr"/>')
                if lines > 1:
                    x = x.replace("<c:separator>; </c:separator>",
                                  "<c:separator>\n</c:separator>")
                # Only the data labels state sz="1000"; the title states 1300 and 1800.
                x = x.replace('<a:defRPr sz="1000"', '<a:defRPr sz="%d"' % size)
                x = re.sub(
                    r'(<a:defRPr sz="%d"(?:(?!</a:defRPr>).)*?<a:latin typeface=")[^"]+(")' % size,
                    r"\g<1>%s\g<2>" % face, x, flags=re.S)
                data = x.encode("utf-8")
            zout.writestr(item, data)
    zin.close()
    return dst


def render(name, wb):
    d = os.path.join(OUT, "r-" + name)
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    subprocess.run(
        ["soffice", "-env:UserInstallation=file://" + os.path.join(OUT, "prof-" + name),
         "--headless", "--convert-to", "pdf", "--outdir", d, wb],
        capture_output=True, timeout=600)
    return os.path.join(d, name + ".pdf")


def runs(pdf):
    txt = subprocess.run([sys.executable, PDFOPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    out = []
    for line in txt.splitlines():
        m = TEXT.match(line)
        if m:
            out.append((float(m.group(1)), float(m.group(2)), float(m.group(3)), m.group(4)))
    return out


def label_runs(rows, size_pt):
    """The runs drawn at the stated label size, which are the labels and nothing else."""
    return [r for r in rows if abs(r[2] - size_pt) <= max(0.25, size_pt * 0.02)]


def columns(rows, size_pt):
    """Group runs into label blocks by x, then order each block top-down."""
    blocks = []
    for r in sorted(rows):
        for b in blocks:
            if abs(b[0][0] - r[0]) < size_pt * 3.0 and \
               min(abs(p[1] - r[1]) for p in b) < size_pt * 6.0:
                b.append(r)
                break
        else:
            blocks.append([r])
    for b in blocks:
        b.sort(key=lambda p: -p[1])
    return blocks


def main():
    os.makedirs(OUT, exist_ok=True)
    missing = []
    table = {}
    for stated, resolved in FACES:
        for size in SIZES:
            size_pt = size / 100.0
            row = {}
            for lines in (1, 4):
                name = "%s-%d-%dl" % (stated.replace(" ", ""), size, lines)
                pdf = render(name, build(name, stated, size, lines))
                if not os.path.exists(pdf):
                    missing.append(name + " (no pdf)")
                    continue
                rows = label_runs(runs(pdf), size_pt)
                blocks = columns(rows, size_pt)
                if lines == 1:
                    # every block is one run; the first baseline is the run's own y
                    firsts = [b[0][1] for b in blocks if len(b) == 1]
                    if len(firsts) < 3:
                        missing.append(name + " (%d one-line blocks)" % len(firsts))
                        continue
                    row["y1"] = sorted(firsts)
                    row["blocks1"] = len(blocks)
                else:
                    gaps = []
                    firsts = []
                    for b in blocks:
                        if len(b) < 2:
                            continue
                        firsts.append(b[0][1])
                        for i in range(len(b) - 1):
                            gaps.append(b[i][1] - b[i + 1][1])
                    if len(gaps) < 3:
                        missing.append(name + " (%d gaps)" % len(gaps))
                        continue
                    row["gaps"] = sorted(gaps)
                    row["y4"] = sorted(firsts)
                    row["blocks4"] = len(blocks)
            table[(stated, size)] = row

    if missing:
        print("REFUSING TO SUMMARISE — cases with no measurable geometry:", file=sys.stderr)
        for m in missing:
            print("   ", m, file=sys.stderr)
        sys.exit(2)

    print("%-10s %7s %9s %9s %9s %9s %9s" %
          ("face", "size", "H(gaps)", "H em", "H(1v4)", "y1(1line)", "blocks"))
    for stated, resolved in FACES:
        for size in SIZES:
            r = table[(stated, size)]
            s = size / 100.0
            g = r["gaps"]
            H = g[len(g) // 2]
            # pair the one-line and four-line first baselines by rank: same anchors, same order
            d = [b - a for a, b in zip(r["y1"], r["y4"])]
            d.sort()
            H2 = d[len(d) // 2] / 1.5
            print("%-10s %7.2f %9.3f %9.4f %9.3f %9.3f %5d/%d" %
                  (resolved, s, H, H / s, H2, r["y1"][len(r["y1"]) // 2],
                   r["blocks1"], r["blocks4"]))

    print("\nascent from the size series (C cancels):  y1(s) = C + H(s)/2 - A(s)")
    for stated, resolved in FACES:
        pts = []
        for size in SIZES:
            r = table[(stated, size)]
            s = size / 100.0
            g = r["gaps"]
            pts.append((s, r["y1"], g[len(g) // 2]))
        # least squares of each ranked anchor separately, then the median slope
        slopes = []
        n = min(len(p[1]) for p in pts)
        for k in range(n):
            xs = [p[0] for p in pts]
            ys = [p[1][k] for p in pts]
            mx = sum(xs) / len(xs)
            my = sum(ys) / len(ys)
            num = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
            den = sum((x - mx) ** 2 for x in xs)
            slopes.append(num / den)
        slopes.sort()
        slope = slopes[len(slopes) // 2]
        hs = [p[2] / p[0] for p in pts]
        hs.sort()
        h = hs[len(hs) // 2]
        print("  %-16s slope dy1/ds = %+7.4f em   median h = %.4f em"
              "   =>  a = h/2 - slope = %.4f em" % (resolved, slope, h, h / 2 - slope))
        print("     per-anchor slopes: %s" % " ".join("%+.4f" % v for v in slopes))


if __name__ == "__main__":
    main()
