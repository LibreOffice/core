#!/usr/bin/env python3
"""What line height does 26.2.4.2 stack a chart label's lines at?

Our chart text measurer answers `ascent + descent + lineGap` from the face's own tables, with a
note saying that is what chart2 uses because a label is a plain text shape.  The corpus witness's
reference rendering stacks two label lines **11.23 pt** apart at 10.01 pt — 1.1219 em — where
Carlito's hhea, OS/2 typo and OS/2 win metrics all give **1.2207**.  This separates "Carlito is
special" from "the formula is wrong": a newline separator forces every label onto two lines, and
the face and the size are varied independently.

Refuses to print unless every case produced two baselines to measure between.
"""
import os, re, shutil, subprocess, sys, zipfile

SRC = "/c/sandbox/workdir/sample-files/sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx"
OUT = "/c/sandbox/workdir/scratch-r59-sheets/lineheight"
PDFOPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"

# (name, latin typeface, label size in hundredths of a point)
CASES = [
    ("calibri-10", "Calibri", 1000),
    ("calibri-20", "Calibri", 2000),
    ("arial-10", "Arial", 1000),
    ("arial-20", "Arial", 2000),
    ("times-10", "Times New Roman", 1000),
    ("courier-10", "Courier New", 1000),
]

TEXT = re.compile(
    r"^text\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt")


def build(name, face, size):
    dst = os.path.join(OUT, name + ".xlsx")
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "xl/charts/chart1.xml":
                x = data.decode("utf-8")
                # Every label on two lines, whatever fits: a newline separator.
                x = x.replace("<c:separator>; </c:separator>",
                              "<c:separator>\n</c:separator>")
                x = x.replace('<a:defRPr sz="1000"', '<a:defRPr sz="%d"' % size)
                # Only the label runs name Calibri at sz=…; the title states 1300/1800.
                x = re.sub(r'(<a:defRPr sz="%d"[^>]*>.*?<a:latin typeface=")[^"]+(")' % size,
                           r"\1" + face + r"\2", x, flags=re.S)
                data = x.encode("utf-8")
            zout.writestr(item, data)
    zin.close()
    return dst


def spacing(pdf):
    txt = subprocess.run([sys.executable, PDFOPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    rows = []
    for line in txt.splitlines():
        m = TEXT.match(line)
        if m:
            rows.append((float(m.group(1)), float(m.group(2)), float(m.group(3)),
                         line.split()[-1] if '"' in line else ""))
    # Label runs are the ones whose size is the stated label size; pair them by x.
    by = {}
    for x, y, sz, _ in rows:
        by.setdefault(round(sz, 1), []).append((x, y))
    best = None
    for sz, pts in by.items():
        if len(pts) < 4:
            continue
        pts.sort()
        gaps = []
        for i in range(len(pts) - 1):
            dx = abs(pts[i + 1][0] - pts[i][0])
            dy = pts[i][1] - pts[i + 1][1]
            if dx < 30 and 0 < dy < 4 * sz:
                gaps.append(dy)
        if gaps and (best is None or len(gaps) > len(best[1])):
            best = (sz, gaps)
    return best


def main():
    os.makedirs(OUT, exist_ok=True)
    prof = os.path.join(OUT, "prof")
    rows, missing = [], []
    for name, face, size in CASES:
        wb = build(name, face, size)
        d = os.path.join(OUT, "r-" + name)
        shutil.rmtree(d, ignore_errors=True)
        os.makedirs(d)
        subprocess.run(["soffice", "-env:UserInstallation=file://" + prof, "--headless",
                        "--convert-to", "pdf", "--outdir", d, wb],
                       capture_output=True, timeout=300)
        pdf = os.path.join(d, name + ".pdf")
        got = spacing(pdf) if os.path.exists(pdf) else None
        if not got:
            missing.append(name)
            continue
        rows.append((name, face, got))

    if missing:
        print("REFUSING TO SUMMARISE — no measurable baseline pair for: %s"
              % ", ".join(missing), file=sys.stderr)
        sys.exit(2)

    print("%-14s %-18s %8s %10s %10s" % ("case", "face", "size", "spacing", "em"))
    for name, face, (sz, gaps) in rows:
        med = sorted(gaps)[len(gaps) // 2]
        print("%-14s %-18s %8.2f %10.2f %10.4f  (n=%d, %s)"
              % (name, face, sz, med, med / sz, len(gaps),
                 " ".join("%.2f" % g for g in sorted(gaps))))


if __name__ == "__main__":
    main()
