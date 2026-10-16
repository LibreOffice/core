"""Render each probe through both LibreOffice binaries and our CLI, and report the row pitch.

Usage: XRAY_OFFICE_CLI=<abs path> python3 measure.py [out-dir ...]   (default: every out*/ there is)

The pitch is the median gap between the page's full-width horizontal rules, off a 300 dpi raster,
which for these ten-row tables is the row height.
"""
import os, subprocess, sys, tempfile
from pathlib import Path
import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
CLI = os.environ["XRAY_OFFICE_CLI"]
BINS = {"24.2": "soffice", "26.2": "/opt/libreoffice26.2/program/soffice"}

def pitch(pdf, dpi=300):
    t = tempfile.mkdtemp()
    subprocess.run(["pdftoppm", "-r", str(dpi), "-f", "1", "-l", "1", "-gray", "-png",
                    str(pdf), os.path.join(t, "p")], capture_output=True)
    files = sorted(os.listdir(t))
    if not files: return None
    a = np.asarray(Image.open(os.path.join(t, files[0])).convert("L")).astype(float)
    dark = (a < 200).mean(axis=1)
    rows = [int(i) for i in np.nonzero(dark > 0.3)[0]]
    if len(rows) < 3: return None
    groups, cur = [], [rows[0]]
    for x in rows[1:]:
        if x - cur[-1] <= 2: cur.append(x)
        else: groups.append(sum(cur) / len(cur)); cur = [x]
    groups.append(sum(cur) / len(cur))
    if len(groups) < 3: return None
    steps = [(groups[i + 1] - groups[i]) / dpi * 72 for i in range(len(groups) - 1)]
    steps.sort()
    return round(steps[len(steps) // 2], 3)

SETS = sys.argv[1:] or sorted(d.name for d in HERE.glob("out*") if d.is_dir())

print(f"{'case':<28}{'24.2':>8}{'26.2':>8}{'ours':>8}")
for src in [s for name in SETS for s in sorted((HERE / name).glob("*.docx"))]:
    got = {}
    for tag, binary in BINS.items():
        folder = HERE / ("render-" + src.parent.name) / tag
        folder.mkdir(parents=True, exist_ok=True)
        out = folder / (src.stem + ".pdf")
        if not out.exists():
            subprocess.run([binary, f"-env:UserInstallation=file://{folder / 'prof'}", "--headless",
                            "--convert-to", "pdf", "--outdir", str(folder), str(src)],
                           capture_output=True, timeout=600)
        got[tag] = pitch(out) if out.exists() else None
    folder = HERE / ("render-" + src.parent.name) / "ours"
    folder.mkdir(parents=True, exist_ok=True)
    out = folder / (src.stem + ".pdf")
    if not out.exists():
        subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(folder)],
                       capture_output=True, timeout=600)
    got["ours"] = pitch(out) if out.exists() else None
    print(f"{src.stem:<28}{str(got['24.2']):>8}{str(got['26.2']):>8}{str(got['ours']):>8}")
