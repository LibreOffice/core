"""Ink against 26.2.4.2 before and after, for a named set of documents.

`before` is the gate's own rendering at the base commit; `after` is this sweep's. The
reference is rendered here with /opt/libreoffice26.2, its Latin metric duplicates and Latin
Noto moved aside, which is the binary this tree is calibrated to.
"""
import os, subprocess, sys, tempfile
from pathlib import Path
import numpy as np
from PIL import Image

CORPUS = Path("/home/user/sample-files")
BEFORE = Path("/home/user/gate-2f47/ours")
AFTER = Path("/home/user/wt-aac/sweep-words/ours")
WORK = Path("/home/user/wt-aac/aac/ba"); WORK.mkdir(parents=True, exist_ok=True)
SOFF = "/opt/libreoffice26.2/program/soffice"

def render_ref(src, out, prof):
    out.mkdir(parents=True, exist_ok=True)
    p = out / (src.stem + ".pdf")
    if p.exists(): return p
    subprocess.run([SOFF, "-env:UserInstallation=file://" + str(prof), "--headless",
                    "--norestore", "--convert-to", "pdf", "--outdir", str(out), str(src)],
                   capture_output=True, timeout=600)
    return p if p.exists() else None

def pages(pdf, dpi=30):
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(["pdftoppm", "-r", str(dpi), "-gray", "-png", str(pdf),
                        os.path.join(t, "p")], capture_output=True)
        return [np.asarray(Image.open(os.path.join(t, n)).convert("L")).astype(float)
                for n in sorted(os.listdir(t))]

def ink(a, b):
    n = min(len(a), len(b))
    if n == 0: return None, None
    vals = []
    for k in range(n):
        x, y = a[k], b[k]
        h, w = min(x.shape[0], y.shape[0]), min(x.shape[1], y.shape[1])
        vals.append(float(np.abs(x[:h, :w] - y[:h, :w]).mean()))
    return sum(vals)/len(vals), max(vals)

print("path\tpagesbefore\tpagesafter\tpagesref\tinkbefore\tinkafter\tworstbefore\tworstafter")
for i, rel in enumerate(l.strip() for l in sys.stdin if l.strip()):
    src = CORPUS / rel
    stem, ext = src.stem, src.suffix[1:].lower()
    key = f"{stem}__{ext}.pdf"
    b, a = BEFORE / key, AFTER / key
    r = render_ref(src, WORK / f"ref{i}", WORK / f"prof{i%2}")
    if not (b.exists() and a.exists() and r):
        print(f"{rel}\tMISSING"); continue
    pb, pa, pr = pages(b), pages(a), pages(r)
    mb, wb = ink(pb, pr)
    ma, wa = ink(pa, pr)
    print(f"{rel}\t{len(pb)}\t{len(pa)}\t{len(pr)}\t{mb:.2f}\t{ma:.2f}\t{wb:.2f}\t{wa:.2f}")
    sys.stdout.flush()
