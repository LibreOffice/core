#!/usr/bin/env python3
"""Plot rectangle of each band probe, from our ink, LibreOffice's ink, and LibreOffice's model."""
import re, subprocess, sys, os, zipfile

OPS = "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
FILL = re.compile(r"^fill\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(#\w+)")
CM = 28.34645669

def plotfill(pdf):
    out = subprocess.run([sys.executable, OPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    best = None
    for ln in out.splitlines():
        m = FILL.match(ln)
        if not m: continue
        if m.group(5).upper() not in ("#D9D9D9",): continue
        x0, y0, x1, y1 = (float(m.group(i)) for i in (1, 2, 3, 4))
        best = (x0, y0, x1, y1)
    return best

def frame(pdf):
    """The chart's own white background box, which both renderers draw at the frame."""
    out = subprocess.run([sys.executable, OPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    for ln in out.splitlines():
        m = FILL.match(ln)
        if not m: continue
        if m.group(5).upper() != "#FFFFFF": continue
        x0, y0, x1, y1 = (float(m.group(i)) for i in (1, 2, 3, 4))
        if (x1 - x0) > 100 and (y1 - y0) > 60 and (x1 - x0) < 700:
            return (x0, y0, x1, y1)
    return None

def region(odp):
    with zipfile.ZipFile(odp) as z:
        for n in z.namelist():
            if n.endswith("content.xml") and n.startswith("Object"):
                s = z.read(n).decode("utf-8")
                m = re.search(r'<chart:coordinate-region svg:x="([\d.]+)cm" svg:y="([\d.]+)cm" '
                              r'svg:width="([\d.]+)cm" svg:height="([\d.]+)cm"', s)
                if m: return tuple(float(m.group(i)) * CM for i in (1, 2, 3, 4))
    return None

names = sorted(os.listdir(sys.argv[1] + "/ours"))
print(f"{'probe':<22}{'below-ours':>11}{'below-ref':>11}{'delta':>8}"
      f"{'above-ours':>12}{'above-ref':>11}{'delta':>8}")
for n in names:
    stem = n[:-4]
    o = plotfill(f"{sys.argv[1]}/ours/{n}")
    r = plotfill(f"{sys.argv[1]}/ref/{n}")
    fo = frame(f"{sys.argv[1]}/ours/{n}")
    fr = frame(f"{sys.argv[1]}/ref/{n}")
    if not (o and r and fo and fr):
        print(f"{stem:<22} missing  ours={bool(o)} ref={bool(r)} fo={bool(fo)} fr={bool(fr)}")
        continue
    bo, br = o[1] - fo[1], r[1] - fr[1]           # plot bottom above frame bottom
    ao, ar = fo[3] - o[3], fr[3] - r[3]           # frame top above plot top
    print(f"{stem:<22}{bo:>11.2f}{br:>11.2f}{bo-br:>8.2f}{ao:>12.2f}{ar:>11.2f}{ao-ar:>8.2f}")
