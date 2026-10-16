#!/usr/bin/env python3
"""Bounding box of the non-white ink on one page of a banked rendering pair, in points.

Answers "is our object drawn wider than the reference's" without opening the page.
"""
import subprocess
import sys
import tempfile
import pathlib

DPI = 100


def box(pdf, page):
    with tempfile.TemporaryDirectory() as d:
        subprocess.run(["pdftoppm", "-r", str(DPI), "-f", str(page), "-l", str(page),
                        "-gray", "-singlefile", pdf, f"{d}/p"], check=True)
        raw = pathlib.Path(f"{d}/p.pgm").read_bytes()
    # P5 header: magic, width height, maxval, then binary
    parts = raw.split(b"\n", 3)
    w, h = (int(x) for x in parts[1].split())
    data = parts[3] if len(parts) > 3 else b""
    xmin, xmax, ymin, ymax = w, -1, h, -1
    for y in range(h):
        row = data[y * w:(y + 1) * w]
        first = None
        for x in range(w):
            if row[x] < 240:
                if first is None:
                    first = x
                last = x
        if first is not None:
            xmin = min(xmin, first)
            xmax = max(xmax, last)
            ymin = min(ymin, y)
            ymax = max(ymax, y)
    k = 72.0 / DPI
    return (xmin * k, ymin * k, xmax * k, ymax * k, (xmax - xmin) * k, (ymax - ymin) * k)


ident, page = sys.argv[1], int(sys.argv[2])
refdir = sys.argv[3] if len(sys.argv) > 3 else "/home/user/gate-2f47/ref"
o = box(f"/home/user/gate-2f47/ours/{ident}.pdf", page)
r = box(f"{refdir}/{ident}.pdf", page)
print(f"        {'x0':>8}{'y0':>8}{'x1':>8}{'y1':>8}{'w':>8}{'h':>8}")
print("ours  " + "".join(f"{v:8.1f}" for v in o))
print("ref   " + "".join(f"{v:8.1f}" for v in r))
print(f"w ratio {o[4]/r[4]:.4f}   h ratio {o[5]/r[5]:.4f}")
