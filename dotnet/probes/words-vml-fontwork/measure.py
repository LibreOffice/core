"""Score the five VML `_x0000_t136` documents against both LibreOffice references.

Usage:  XRAY_OFFICE_CLI=<abs path> TAG=before python3 measure.py <out-dir>

Renders each document with soffice 24.2, /opt/libreoffice26.2 and our CLI, then reports
per-page mean absolute grey difference at 100 dpi and per-page `pdftotext` word counts.
Reference renders are cached; ours are re-rendered per TAG so two builds compare without
re-rendering either reference.
"""
import os, subprocess, sys, tempfile
from pathlib import Path
import numpy as np
from PIL import Image

DOCS = [
    "words/ceiling-001/docx/ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx",
    "words/extra-001/docx/ABCD-SDE-23-00 - Avionic System Description - 17.02.16 - v1.docx",
    "words/extra-001/docx/ABCD-WB-08-00 Weight and Balance Report - v1 08.03.16.docx",
    "words/done-015/docx/DOA_Template_Form_Type_Certification_Programme.docx",
    "words/done-012/docx/technical-architecture.docx",
]

CORPUS = Path(os.environ.get("CORPUS", "/home/user/sample-files"))
CLI = os.environ["XRAY_OFFICE_CLI"]
LO24 = os.environ.get("LO24", "soffice")
LO26 = os.environ.get("LO26", "/opt/libreoffice26.2/program/soffice")
TAG = os.environ.get("TAG", "run")


def page(pdf, n, dpi=100):
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(["pdftoppm", "-r", str(dpi), "-f", str(n), "-l", str(n), "-gray", "-png",
                        str(pdf), os.path.join(t, "p")], capture_output=True)
        m = sorted(os.listdir(t))
        return None if not m else np.asarray(Image.open(os.path.join(t, m[0])).convert("L")).astype(float)


def pages(pdf):
    r = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True)
    return next((int(l.split()[1]) for l in r.stdout.splitlines() if l.startswith("Pages:")), 0)


def words(pdf, n=None):
    cmd = ["pdftotext"]
    if n is not None:
        cmd += ["-f", str(n), "-l", str(n)]
    cmd += [str(pdf), "-"]
    r = subprocess.run(cmd, capture_output=True, text=True)
    return len(r.stdout.split())


def render_lo(binary, src, folder):
    folder.mkdir(parents=True, exist_ok=True)
    out = folder / (src.stem + ".pdf")
    if not out.exists():
        subprocess.run([binary, f"-env:UserInstallation=file://{folder / 'prof'}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(folder), str(src)],
                       capture_output=True, timeout=1800)
    return out


def render_ours(src, folder):
    folder.mkdir(parents=True, exist_ok=True)
    out = folder / (src.stem + ".pdf")
    if out.exists():
        out.unlink()
    subprocess.run([CLI, "render", str(src), "--outdir", str(folder)],
                   capture_output=True, timeout=1800)
    return out


def main():
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    only = sys.argv[2:]

    for rel in DOCS:
        src = CORPUS / rel
        if only and not any(s.lower() in src.name.lower() for s in only):
            continue
        key = src.stem[:28].replace(" ", "_")
        o = render_lo(LO24, src, out / "ref24" / key)
        n = render_lo(LO26, src, out / "ref26" / key)
        m = render_ours(src, out / TAG / key)

        print(f"\n=== {src.name}")
        print(f"    pages  24.2 {pages(o)}  26.2 {pages(n)}  ours {pages(m)}")
        print(f"    words  24.2 {words(o)}  26.2 {words(n)}  ours {words(m)}")
        d24, d26 = [], []
        for i in range(1, max(1, min(pages(o), pages(n), pages(m))) + 1):
            a, b, c = page(o, i), page(n, i), page(m, i)
            if a is None or b is None or c is None:
                continue

            def diff(x, y):
                h, w = min(x.shape[0], y.shape[0]), min(x.shape[1], y.shape[1])
                return round(float(np.abs(x[:h, :w] - y[:h, :w]).mean()), 2)

            i24, i26 = diff(c, a), diff(c, b)
            d24.append(i24)
            d26.append(i26)
            print(f"    page {i:3d}  ink24 {i24:6.2f}  ink26 {i26:6.2f}  "
                  f"words {words(o, i):5d} / {words(n, i):5d} / {words(m, i):5d}")
        if d24:
            print(f"    MEAN   ink24 {sum(d24)/len(d24):6.3f}  ink26 {sum(d26)/len(d26):6.3f}  "
                  f"over {len(d24)} pages")


main()
