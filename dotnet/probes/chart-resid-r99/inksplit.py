#!/usr/bin/env python3
"""Ink pixels ours-only / ref-only inside a page band, split by whether they fall in a text box.

Usage: inksplit.py OURS.pdf REF.pdf PAGE [dpi]
The text boxes are every drawn text span of *either* rendering, grown by a margin, which is the
set a label-placement fix could possibly move.
"""
import sys
import numpy as np
import pymupdf

def page_arr(path, pageno, dpi):
    d = pymupdf.open(path)
    p = d[pageno - 1]
    pix = p.get_pixmap(dpi=dpi, colorspace=pymupdf.csGRAY)
    a = np.frombuffer(pix.samples, dtype=np.uint8).reshape(pix.height, pix.width).astype(int)
    spans = [s["bbox"] for b in p.get_text("dict")["blocks"] for l in b.get("lines", [])
             for s in l["spans"]]
    return a, spans, p.rect

def main():
    ours, ref, pageno = sys.argv[1], sys.argv[2], int(sys.argv[3])
    dpi = int(sys.argv[4]) if len(sys.argv) > 4 else 120
    a, sa, rect = page_arr(ours, pageno, dpi)
    b, sb, _ = page_arr(ref, pageno, dpi)
    h = min(a.shape[0], b.shape[0]); w = min(a.shape[1], b.shape[1])
    a, b = a[:h, :w], b[:h, :w]
    ink_a, ink_b = a < 250, b < 250
    scale = dpi / 72.0
    text = np.zeros((h, w), dtype=bool)
    for x0, y0, x1, y1 in sa + sb:
        i0, i1 = max(0, int(y0 * scale) - 2), min(h, int(y1 * scale) + 3)
        j0, j1 = max(0, int(x0 * scale) - 2), min(w, int(x1 * scale) + 3)
        text[i0:i1, j0:j1] = True
    only_a, only_b = ink_a & ~ink_b, ink_b & ~ink_a
    total = h * w
    for name, m in (("ours-only", only_a), ("ref-only", only_b)):
        print(f"{name}: {m.sum():8d} px  ({100.0 * m.sum() / total:5.2f}% of page)"
              f"   in a text box {int((m & text).sum()):7d}"
              f"   outside {int((m & ~text).sum()):8d}")

main()
