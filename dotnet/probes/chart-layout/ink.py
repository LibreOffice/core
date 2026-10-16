"""Ink between an XRay.Office rendering and 26.2.4.2's, per document, over every shared page.

Ink is the mean absolute grey difference at 30 dpi, page for page, reported as the mean over
pages and as the worst page — the same measure as `probes/words-apo-table/inkcheck.py`, from
which this is taken. The one change is that the reference render is *cached* under
`--refdir`, because this round measures the same twenty documents several times over and
`soffice` is the whole cost.

    ink.py --cli <XRay.Office.Cli> --work <dir> --refdir <dir> < list-of-corpus-relative-paths
"""
import argparse, os, subprocess, sys, tempfile
from pathlib import Path
import numpy as np
from PIL import Image

CORPUS = Path("/home/user/sample-files")
SOFF = "/opt/libreoffice26.2/program/soffice"

ap = argparse.ArgumentParser()
ap.add_argument("--cli", required=True)
ap.add_argument("--work", required=True)
ap.add_argument("--refdir", required=True)
ap.add_argument("--dpi", type=int, default=30)
args = ap.parse_args()

WORK = Path(args.work); WORK.mkdir(parents=True, exist_ok=True)
REF = Path(args.refdir); REF.mkdir(parents=True, exist_ok=True)


def render_ours(src, out):
    out.mkdir(parents=True, exist_ok=True)
    p = out / (src.stem + ".pdf")
    if p.exists():
        p.unlink()
    subprocess.run([args.cli, "render", str(src), "--format", "pdf", "--outdir", str(out)],
                   capture_output=True, timeout=600)
    return p if p.exists() else None


def render_ref(src, key):
    out = REF / key
    p = out / (src.stem + ".pdf")
    if p.exists():
        return p
    out.mkdir(parents=True, exist_ok=True)
    prof = REF / ("prof" + key[-1])
    subprocess.run([SOFF, "-env:UserInstallation=file://" + str(prof), "--headless",
                    "--norestore", "--convert-to", "pdf", "--outdir", str(out), str(src)],
                   capture_output=True, timeout=600)
    return p if p.exists() else None


def pages(pdf, dpi):
    with tempfile.TemporaryDirectory(dir=str(WORK)) as t:
        subprocess.run(["pdftoppm", "-r", str(dpi), "-gray", "-png", str(pdf),
                        os.path.join(t, "p")], capture_output=True)
        return [np.asarray(Image.open(os.path.join(t, n)).convert("L")).astype(float)
                for n in sorted(os.listdir(t))]


def npages(pdf):
    r = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True)
    return next((int(l.split()[1]) for l in r.stdout.splitlines() if l.startswith("Pages:")), 0)


rows = [l.strip() for l in sys.stdin if l.strip() and not l.startswith("#")]
print("path\tourpages\trefpages\tinkmean\tinkworst\tworstpage")
for i, rel in enumerate(rows):
    src = CORPUS / rel
    if not src.exists():
        print(f"{rel}\tMISSING"); sys.stdout.flush(); continue
    o = render_ours(src, WORK / f"ours{i}")
    r = render_ref(src, f"r{i}")
    if o is None or r is None:
        print(f"{rel}\t{'-' if o is None else npages(o)}\t{'-' if r is None else npages(r)}"
              f"\tRENDER-FAILED\t-\t-"); sys.stdout.flush(); continue
    op, rp = pages(o, args.dpi), pages(r, args.dpi)
    n = min(len(op), len(rp))
    vals = []
    for k in range(n):
        a, b = op[k], rp[k]
        h, w = min(a.shape[0], b.shape[0]), min(a.shape[1], b.shape[1])
        vals.append(float(np.abs(a[:h, :w] - b[:h, :w]).mean()))
    if not vals:
        print(f"{rel}\t{len(op)}\t{len(rp)}\tNO-PAGES\t-\t-"); sys.stdout.flush(); continue
    worst = max(range(len(vals)), key=lambda k: vals[k])
    print(f"{rel}\t{len(op)}\t{len(rp)}\t{sum(vals)/len(vals):.2f}\t{vals[worst]:.2f}\t{worst+1}")
    sys.stdout.flush()
