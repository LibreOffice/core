"""Which of the worst words documents are real defects, and which are the 24.2-vs-26.2 gap?

Renders the top N by first-page ink with the 26.2.4.2 tarball -- the version this tree targets --
and scores our output against both references.
"""
import os, subprocess, sys, tempfile, statistics
from pathlib import Path
import numpy as np
from PIL import Image

SP = Path("/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad")
SWEEP = SP / "sweep-rh"
OUT = SP / "ref262"; OUT.mkdir(exist_ok=True)
CORPUS = Path("/home/user/sample-files")
TOP = int(sys.argv[1]) if len(sys.argv) > 1 else 30

def page(pdf, dpi=30):
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(["pdftoppm", "-r", str(dpi), "-f", "1", "-l", "1", "-gray", "-png",
                        str(pdf), os.path.join(t, "p")], capture_output=True)
        m = sorted(os.listdir(t))
        if not m: return None
        return np.asarray(Image.open(os.path.join(t, m[0])).convert("L")).astype(float)

def ink(a, b):
    x, y = page(a), page(b)
    if x is None or y is None: return None
    h, w = min(x.shape[0], y.shape[0]), min(x.shape[1], y.shape[1])
    return round(float(np.abs(x[:h, :w] - y[:h, :w]).mean()), 2)

def pages(pdf):
    r = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True)
    return next((int(l.split()[1]) for l in r.stdout.splitlines() if l.startswith("Pages:")), 0)

# rank by ink against the 24.2 gate, which is what the sweep holds
rows = []
for n in sorted(os.listdir(SWEEP / "ours")):
    o, r = SWEEP / "ours" / n, SWEEP / "ref" / n
    if not r.exists(): continue
    v = ink(r, o)
    if v is not None: rows.append((v, n))
rows.sort(reverse=True)

# find each source file by the sweep's identity key
index = {}
for p in CORPUS.rglob("*"):
    if p.is_file():
        index.setdefault(f"{p.stem}__{p.suffix[1:].lower()}.pdf", p)

print(f"{'ink24':>6} {'ink26':>6} {'verdict':>10}  {'pp 24/26/ours':>14}  document")
for v, n in rows[:TOP]:
    src = index.get(n)
    if src is None:
        print(f"{v:6.2f} {'-':>6} {'no source':>10}  {'':>14}  {n[:52]}")
        continue
    here = OUT / n[:-4]; here.mkdir(exist_ok=True)
    new = here / (src.stem + ".pdf")
    if not new.exists():
        subprocess.run(["/opt/libreoffice26.2/program/soffice",
                        f"-env:UserInstallation=file://{here / 'prof'}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(here), str(src)],
                       capture_output=True, timeout=600)
    if not new.exists():
        print(f"{v:6.2f} {'-':>6} {'26.2 failed':>10}  {'':>14}  {n[:52]}")
        continue
    v26 = ink(new, SWEEP / "ours" / n)
    verdict = "VERSION" if v26 is not None and v26 < v * 0.5 else ("ours" if v26 is not None and v26 > v * 0.9 else "mixed")
    pp = f"{pages(SWEEP / 'ref' / n)}/{pages(new)}/{pages(SWEEP / 'ours' / n)}"
    print(f"{v:6.2f} {v26 if v26 is not None else '-':>6} {verdict:>10}  {pp:>14}  {n[:52]}", flush=True)
