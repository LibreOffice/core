#!/usr/bin/env python3
"""Stack reference / candidate renders into one reviewable image.

Vertical stacking is wrong for a wide comparison and side-by-side is wrong for a
tall one; this puts the three engines in a row with labels, which is how a reader
compares a page against the reference at a glance.
"""
import sys, pathlib
import numpy as np, fitz
from PIL import Image, ImageDraw

sys.path.insert(0, "/data/bench/scripts")
import metrics as M

BENCH = pathlib.Path("/data/bench")


def pdf_gray(path, index, dpi=150):
    d = fitz.open(path)
    if index >= d.page_count:
        return None
    pm = d.load_page(index).get_pixmap(dpi=dpi, colorspace=fitz.csGRAY, alpha=False)
    a = np.frombuffer(pm.samples, np.uint8).reshape(pm.height, pm.width).copy()
    d.close()
    return a


def pdf_rgb(path, index, dpi=150):
    d = fitz.open(path)
    if index >= d.page_count:
        return None
    pm = d.load_page(index).get_pixmap(dpi=dpi, alpha=False)
    im = Image.frombytes("RGB", (pm.width, pm.height), pm.samples)
    d.close()
    return im


def panel(im, target_h, label, sub=""):
    if im is None:
        im = Image.new("RGB", (int(target_h * 0.72), target_h), (238, 238, 240))
        d = ImageDraw.Draw(im)
        d.text((14, 14), "no output", fill=(150, 30, 30))
    w = max(1, int(im.width * target_h / im.height))
    im = im.resize((w, target_h), Image.LANCZOS)
    out = Image.new("RGB", (im.width, target_h + 34), (255, 255, 255))
    out.paste(im, (0, 34))
    d = ImageDraw.Draw(out)
    d.rectangle([0, 0, out.width - 1, 33], fill=(30, 32, 38))
    d.text((8, 6), label, fill=(255, 255, 255))
    if sub:
        d.text((8, 19), sub, fill=(170, 175, 190))
    d.rectangle([0, 34, out.width - 1, out.height - 1], outline=(205, 208, 214))
    return out


def build(rid, page, dest, labels, height=760):
    ims = []
    ref_pdf = BENCH / "lo" / rid / "out.pdf"
    ims.append((pdf_rgb(ref_pdf, page - 1), labels[0], ""))
    pl_pdf = BENCH / "pl" / rid / "out.pdf"
    ims.append((pdf_rgb(pl_pdf, page - 1) if pl_pdf.exists() else None, labels[1], ""))
    wv = BENCH / "wv" / rid / f"page-{page}.png"
    ims.append((Image.open(wv).convert("RGB") if wv.exists() else None, labels[2], ""))
    panels = [panel(im, height, lab, sub) for im, lab, sub in ims]
    gap = 12
    W = sum(p.width for p in panels) + gap * (len(panels) - 1)
    H = max(p.height for p in panels)
    out = Image.new("RGB", (W, H), (255, 255, 255))
    x = 0
    for p in panels:
        out.paste(p, (x, 0))
        x += p.width + gap
    out.save(dest)
    return dest


if __name__ == "__main__":
    rid, page, dest = sys.argv[1], int(sys.argv[2]), sys.argv[3]
    build(rid, page, dest, ["LibreOffice (reference)", "XRay.Office", "WASM viewer"])
    print(dest)
