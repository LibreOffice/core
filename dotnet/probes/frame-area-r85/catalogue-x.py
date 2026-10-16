#!/usr/bin/env python3
"""The horizontal extent of the ink on one page of the WordArt catalogue.

`XRay.Office.WordProcessing/TODO.md` recorded `wp:effectExtent` as unread on an inline
drawing's *horizontal* position, quoting page 3 of
`words/drawingset-001/docx/WordArt_Shapes_Arrows_Catalog1.docx` at 200 dpi: ours
229.68..359.64 pt against the reference's 240.84..370.80, "the same 10.8 pt as their
`wp:effectExtent`". This reads that same quantity back, against both installed
references, so the entry can be checked rather than believed.

Usage:  XRAY_OFFICE_CLI=<abs path> python3 catalogue-x.py <workdir> [page|all]
"""
import os, subprocess, sys
from pathlib import Path

import numpy as np
from PIL import Image

DOC = Path(os.environ.get("CORPUS", "/home/user/sample-files")) / \
    "words/drawingset-001/docx/WordArt_Shapes_Arrows_Catalog1.docx"
CLI = os.environ["XRAY_OFFICE_CLI"]
LO24 = os.environ.get("LO24", "/usr/bin/soffice")
LO26 = os.environ.get("LO26", "/opt/libreoffice26.2/program/soffice")
DPI = 200


def render_lo(soffice, doc, outdir):
    outdir.mkdir(parents=True, exist_ok=True)
    pdf = outdir / (doc.stem + ".pdf")
    if not pdf.exists():
        # A profile path is keyed on a hex digest because `soffice` truncates
        # `-env:UserInstallation` at the first space.
        prof = outdir / "prof"
        subprocess.run([soffice, f"-env:UserInstallation=file://{prof}", "--headless",
                        "--norestore", "--convert-to", "pdf", "--outdir", str(outdir), str(doc)],
                       capture_output=True, timeout=600)
    return pdf


def render_ours(doc, outdir):
    outdir.mkdir(parents=True, exist_ok=True)
    pdf = outdir / (doc.stem + ".pdf")
    if pdf.exists():
        pdf.unlink()
    subprocess.run([CLI, "render", str(doc), "--format", "pdf", "--outdir", str(outdir)],
                   capture_output=True, timeout=1800)
    return pdf


def columns(pdf, page):
    """The first and last inked column of a page, in points from the left edge."""
    stem = str(pdf)[:-4] + f".p{page}"
    subprocess.run(["pdftoppm", "-r", str(DPI), "-png", "-f", str(page), "-l", str(page),
                    "-singlefile", str(pdf), stem], check=True, capture_output=True)
    a = np.asarray(Image.open(stem + ".png").convert("L"), dtype=np.int16)
    inked = np.flatnonzero((a < 200).any(axis=0))
    os.unlink(stem + ".png")
    if inked.size == 0:
        return None
    return (inked[0] * 72.0 / DPI, (inked[-1] + 1) * 72.0 / DPI)


def pagecount(pdf):
    out = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True).stdout
    m = [l for l in out.splitlines() if l.startswith("Pages:")]
    return int(m[0].split()[1]) if m else 0


def main():
    work = Path(sys.argv[1])
    which = sys.argv[2] if len(sys.argv) > 2 else "3"
    pdfs = {"24.2": render_lo(LO24, DOC, work / "ref24"),
            "26.2": render_lo(LO26, DOC, work / "ref26"),
            "ours": render_ours(DOC, work / "ours")}
    counts = {k: pagecount(v) for k, v in pdfs.items()}
    print(f"{DOC.name}: pages " + "  ".join(f"{k} {v}" for k, v in counts.items()))
    pages = range(1, min(counts.values()) + 1) if which == "all" else [int(which)]
    print(f"ink columns at {DPI} dpi, in points")
    print(f"{'page':>5s} " + " ".join(f"{k+' xMin':>10s} {k+' xMax':>10s}" for k in pdfs))
    off = 0
    for page in pages:
        cols = {k: columns(v, page) for k, v in pdfs.items()}
        cells = []
        for k in pdfs:
            c = cols[k]
            cells.append(f"{c[0]:10.2f} {c[1]:10.2f}" if c else f"{'-':>10s} {'-':>10s}")
        print(f"{page:5d} " + " ".join(cells))
        a, b = cols["ours"], cols["26.2"]
        if a and b and (abs(a[0] - b[0]) > 0.4 or abs(a[1] - b[1]) > 0.4):
            off += 1
    if which == "all":
        print(f"pages whose ink columns differ from 26.2 by more than the raster quantum: "
              f"{off} of {len(list(pages))}")


main()
