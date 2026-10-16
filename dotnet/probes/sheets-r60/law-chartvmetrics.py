#!/usr/bin/env python3
"""The law, fitted to nothing: a chart text shape's line pitch is an integer count of 96-dpi
pixels, and so is its ascent.

`probe-chartvmetrics2.py` renders the series; this reads its cached PDFs back and tests four
candidate laws against all of them at once.  Nothing here is tuned — the only free choice is the
device resolution, and 96 dpi is both the value that makes every pitch an integer and the default
`VirtualDevice` resolution a headless LibreOffice builds `chart2`'s reference device at.

    hpx  = round(size_pt * dpi / 72)                     the font's height in device pixels
    apx  = round(hheaAscender  / upem * hpx)             ascent, rounded to a whole pixel
    dpx  = round(-hheaDescender / upem * hpx)            descent, likewise
    pitch = (apx + dpx) * 72 / dpi                       and the line gap is NOT in it

The competing laws are the one we ship (`ascent + descent + lineGap`, scaled continuously), the
same without the gap, and the same with the gap but pixel-rounded.  Liberation Sans and Liberation
Serif have a non-zero `hhea` line gap and Carlito's is zero, so the three faces separate the gap
term outright; the pixel rounding is separated by the sizes where `size * 4/3` is not an integer.

Each rendering carries its own uniform chart scale — the drawing is made at a reference size and
scaled onto the sheet — so every measured length is divided by that rendering's own
`drawn_em / stated_em` before it is compared.  That factor is read from the same PDF and is never
larger than 0.7%.

Refuses to print unless every cached case is present.
"""
import os
import re
import struct
import subprocess
import sys

OUT = "/c/sandbox/workdir/scratch-r60-sheets/vmetrics2"
PDFOPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"
DPI = 96.0

FACES = [
    ("Calibri", "Carlito", "/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf"),
    ("Arial", "Liberation Sans",
     "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"),
    ("Times New Roman", "Liberation Serif",
     "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf"),
]
SIZES = [600, 800, 1000, 1100, 1200, 1400, 1600, 1800, 2000, 2400, 2800, 3200, 4000]
SIZES_B = [600, 800, 1000, 1100, 1200, 1400, 1600, 1800, 2000, 2400]

TEXT = re.compile(r"^text\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt")


def hhea(path):
    d = open(path, "rb").read()
    n = struct.unpack(">H", d[4:6])[0]
    t = {}
    for i in range(n):
        off = 12 + 16 * i
        tag = d[off:off + 4].decode("latin-1")
        o, l = struct.unpack(">II", d[off + 8:off + 16])
        t[tag] = d[o:o + l]
    upem = struct.unpack(">H", t["head"][18:20])[0]
    asc, desc, gap = struct.unpack(">hhh", t["hhea"][4:10])
    return upem, asc, -desc, gap


def runs(name, lo=341.0, hi=625.0, size_pt=None):
    pdf = os.path.join(OUT, "r-" + name, name + ".pdf")
    if not os.path.exists(pdf):
        return None
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
        if size_pt is not None and abs(sz - size_pt) > max(0.3, size_pt * 0.03):
            continue
        out.append((x, y, sz))
    return out


def pixels(size_pt, upem, asc, desc, gap):
    hpx = round(size_pt * DPI / 72.0)
    apx = round(asc / upem * hpx)
    dpx = round(desc / upem * hpx)
    gpx = round(gap / upem * hpx)
    return hpx, apx, dpx, gpx


def main():
    missing = []
    print("A. line pitch — four candidate laws against 26.2.4.2's own stacking")
    print("%-16s %6s %6s %8s %8s | %8s %8s %8s %8s" %
          ("face", "size", "hpx", "measured", "px", "px law", "px+gap", "cont.", "cont+gap"))
    err = {"px": [], "pxgap": [], "cont": [], "contgap": []}
    for stated, resolved, path in FACES:
        upem, asc, desc, gap = hhea(path)
        for size in SIZES:
            s = size / 100.0
            one = runs("t-%s-%d-1" % (stated.replace(" ", ""), size), size_pt=s)
            three = runs("t-%s-%d-3" % (stated.replace(" ", ""), size), size_pt=s)
            if not one or not three or len(one) != 1 or len(three) != 3:
                missing.append("t-%s-%d" % (stated, size))
                continue
            three.sort(key=lambda r: -r[1])
            scale = three[0][2] / s
            gaps = sorted(three[i][1] - three[i + 1][1] for i in range(2))
            measured = gaps[len(gaps) // 2] / scale
            hpx, apx, dpx, gpx = pixels(s, upem, asc, desc, gap)
            cand = {
                "px": (apx + dpx) * 72.0 / DPI,
                "pxgap": (apx + dpx + gpx) * 72.0 / DPI,
                "cont": (asc + desc) / upem * s,
                "contgap": (asc + desc + gap) / upem * s,
            }
            for k, v in cand.items():
                err[k].append(abs(v - measured))
            print("%-16s %6.2f %6d %8.3f %8.2f | %8.3f %8.3f %8.3f %8.3f" %
                  (resolved, s, hpx, measured, measured * DPI / 72.0,
                   cand["px"], cand["pxgap"], cand["cont"], cand["contgap"]))

    if missing:
        print("REFUSING TO SUMMARISE — cached renderings absent: %s" % ", ".join(missing),
              file=sys.stderr)
        sys.exit(2)

    print("\n%-10s %10s %10s %10s" % ("law", "max err", "mean err", "exact"))
    for k in ("px", "pxgap", "cont", "contgap"):
        e = err[k]
        print("%-10s %10.4f %10.4f %6d/%d" %
              (k, max(e), sum(e) / len(e), sum(1 for v in e if v < 0.01), len(e)))

    print("\nB. ascent — a CENTER label's block centre must come out size-independent")
    print("   y1 = C + H/2 - A, so C = y1 - (H/2 - A) and the spread of C is the residual.")
    for stated, resolved, path in FACES:
        upem, asc, desc, gap = hhea(path)
        cs = []
        print("  %s" % resolved)
        for size in SIZES_B:
            s = size / 100.0
            rows = runs("a-%s-%d" % (stated.replace(" ", ""), size), hi=570.0, size_pt=s)
            if not rows or len(rows) != 5:
                missing.append("a-%s-%d" % (stated, size))
                continue
            scale = rows[0][2] / s
            hpx, apx, dpx, gpx = pixels(s, upem, asc, desc, gap)
            H = (apx + dpx) * 72.0 / DPI
            A = apx * 72.0 / DPI
            adjust = (H / 2.0 - A) * scale
            ys = sorted(r[1] for r in rows)
            cs.append([y - adjust for y in ys])
            # the continuous model we ship today, for contrast
            Hc = (asc + desc + gap) / upem * s
            Ac = asc / upem * s
            print("     %5.2f pt  hpx %3d  H %7.3f  A %7.3f   C = %s"
                  % (s, hpx, H, A, " ".join("%8.3f" % v for v in [y - adjust for y in ys])))
        if not cs:
            continue
        n = min(len(c) for c in cs)
        spread = [max(c[k] for c in cs) - min(c[k] for c in cs) for k in range(n)]
        print("     pixel law:      per-anchor spread of C = %s  (max %.3f pt)"
              % (" ".join("%.3f" % v for v in spread), max(spread)))
        cs2 = []
        for size in SIZES_B:
            s = size / 100.0
            rows = runs("a-%s-%d" % (stated.replace(" ", ""), size), hi=570.0, size_pt=s)
            scale = rows[0][2] / s
            Hc = (asc + desc + gap) / upem * s
            Ac = asc / upem * s
            adjust = (Hc / 2.0 - Ac) * scale
            cs2.append([y - adjust for y in sorted(r[1] for r in rows)])
        spread2 = [max(c[k] for c in cs2) - min(c[k] for c in cs2) for k in range(n)]
        print("     what we ship:   per-anchor spread of C = %s  (max %.3f pt)"
              % (" ".join("%.3f" % v for v in spread2), max(spread2)))

    if missing:
        print("REFUSING TO SUMMARISE — cached renderings absent: %s" % ", ".join(missing),
              file=sys.stderr)
        sys.exit(2)


if __name__ == "__main__":
    main()
