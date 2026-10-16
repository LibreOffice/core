#!/usr/bin/env python3
"""Where the red band lands, per fixture, through both references and through X-Ray.Office.

The band is the only red ink on the page, so its rectangle separates cleanly from the body
line and from the running heads without any assumption about how wide or how dark it is.

Usage:  XRAY_OFFICE_CLI=<abs path> python3 measure-relfrom.py <fixturedir> <workdir> [name...]
"""
import hashlib, os, subprocess, sys
from pathlib import Path

import numpy as np
from PIL import Image

DPI = 288
LO24 = os.environ.get("LO24", "/usr/bin/soffice")
LO26 = os.environ.get("LO26", "/opt/libreoffice26.2/program/soffice")
CLI = os.environ["XRAY_OFFICE_CLI"]


def render_lo(soffice, doc, outdir):
    outdir.mkdir(parents=True, exist_ok=True)
    pdf = outdir / (doc.stem + ".pdf")
    if not pdf.exists():
        # `soffice` truncates `-env:UserInstallation` at the first space, so the profile path
        # is keyed on a hex digest rather than on the fixture's name.
        prof = outdir / ("p" + hashlib.sha1(str(doc).encode()).hexdigest()[:12])
        subprocess.run([soffice, f"-env:UserInstallation=file://{prof}", "--headless",
                        "--norestore", "--convert-to", "pdf", "--outdir", str(outdir), str(doc)],
                       capture_output=True, timeout=300)
    return pdf


def render_ours(doc, outdir):
    outdir.mkdir(parents=True, exist_ok=True)
    pdf = outdir / (doc.stem + ".pdf")
    if pdf.exists():
        pdf.unlink()
    subprocess.run([CLI, "render", str(doc), "--format", "pdf", "--outdir", str(outdir)],
                   capture_output=True, timeout=300)
    return pdf


def band(pdf):
    """The red rectangle's (x, y, right, bottom) in points from the page's top-left."""
    if not pdf.exists():
        return None
    stem = str(pdf)[:-4] + ".r"
    subprocess.run(["pdftoppm", "-r", str(DPI), "-png", "-f", "1", "-l", "1", "-singlefile",
                    str(pdf), stem], check=True, capture_output=True)
    a = np.asarray(Image.open(stem + ".png").convert("RGB"), dtype=np.int16)
    os.unlink(stem + ".png")
    red = (a[:, :, 0] > 180) & (a[:, :, 1] < 100) & (a[:, :, 2] < 100)
    rows, cols = np.flatnonzero(red.any(axis=1)), np.flatnonzero(red.any(axis=0))
    if rows.size == 0:
        return None
    k = 72.0 / DPI
    return (cols[0] * k, rows[0] * k, (cols[-1] + 1) * k, (rows[-1] + 1) * k)


def main():
    fx, work = Path(sys.argv[1]), Path(sys.argv[2])
    only = sys.argv[3:]
    print(f"{'fixture':24s} {'who':5s} {'x':>8s} {'y':>8s} {'right':>8s} {'bottom':>8s}")
    for doc in sorted(fx.glob("*.docx")):
        if only and doc.stem not in only:
            continue
        rows = [("24.2", band(render_lo(LO24, doc, work / "ref24"))),
                ("26.2", band(render_lo(LO26, doc, work / "ref26"))),
                ("ours", band(render_ours(doc, work / "ours")))]
        for who, b in rows:
            cells = "  ".join(f"{v:8.2f}" for v in b) if b else f"{'(no band)':>36s}"
            print(f"{doc.stem:24s} {who:5s} {cells}")
        print()


main()
