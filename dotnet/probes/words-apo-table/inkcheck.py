"""Ink between our rendering and 26.2.4.2's, per document, over every shared page.

Ink is the mean absolute grey difference at 30 dpi, page for page, reported as the mean over
pages and as the worst page. It is the same measure `probes/words-version-screen/screen.py`
uses, widened from page 1 to the whole document because the rounds under test moved charts and
faces that sit well past the first page.
"""
import os, subprocess, sys, tempfile
from pathlib import Path
import numpy as np
from PIL import Image

CORPUS = Path("/home/user/sample-files")
CLI = "/home/user/wt-aac/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
SOFF = "/opt/libreoffice26.2/program/soffice"
WORK = Path(sys.argv[1])
WORK.mkdir(parents=True, exist_ok=True)

def render_ours(src, out):
    out.mkdir(parents=True, exist_ok=True)
    subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(out)],
                   capture_output=True, timeout=300)
    p = out / (src.stem + ".pdf")
    return p if p.exists() else None

def render_ref(src, out, prof):
    out.mkdir(parents=True, exist_ok=True)
    subprocess.run([SOFF, "-env:UserInstallation=file://" + str(prof), "--headless",
                    "--norestore", "--convert-to", "pdf", "--outdir", str(out), str(src)],
                   capture_output=True, timeout=300)
    p = out / (src.stem + ".pdf")
    return p if p.exists() else None

def pages(pdf, dpi=30):
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(["pdftoppm", "-r", str(dpi), "-gray", "-png", str(pdf),
                        os.path.join(t, "p")], capture_output=True)
        out = []
        for n in sorted(os.listdir(t)):
            out.append(np.asarray(Image.open(os.path.join(t, n)).convert("L")).astype(float))
        return out

def npages(pdf):
    r = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True)
    return next((int(l.split()[1]) for l in r.stdout.splitlines() if l.startswith("Pages:")), 0)

rows = [l.strip() for l in sys.stdin if l.strip() and not l.startswith("#")]
print("path\tourpages\trefpages\tinkmean\tinkworst\tworstpage")
for i, rel in enumerate(rows):
    src = CORPUS / rel
    if not src.exists():
        print(f"{rel}\tMISSING"); continue
    o = render_ours(src, WORK / f"ours{i}")
    r = render_ref(src, WORK / f"ref{i}", WORK / f"prof{i%3}")
    if o is None or r is None:
        print(f"{rel}\t{'-' if o is None else npages(o)}\t{'-' if r is None else npages(r)}\tRENDER-FAILED\t-\t-")
        continue
    op, rp = pages(o), pages(r)
    n = min(len(op), len(rp))
    vals = []
    for k in range(n):
        a, b = op[k], rp[k]
        h, w = min(a.shape[0], b.shape[0]), min(a.shape[1], b.shape[1])
        vals.append(float(np.abs(a[:h, :w] - b[:h, :w]).mean()))
    if not vals:
        print(f"{rel}\t{len(op)}\t{len(rp)}\tNO-PAGES\t-\t-"); continue
    worst = max(range(len(vals)), key=lambda k: vals[k])
    print(f"{rel}\t{len(op)}\t{len(rp)}\t{sum(vals)/len(vals):.2f}\t{vals[worst]:.2f}\t{worst+1}")
    sys.stdout.flush()
