#!/usr/bin/env python3
"""The pitch a sheet's rows are actually drawn at, read off a rendered PDF.

    row-pitch.py <pdf> [<pdf> ...]

O39 is a row-height defect and the row that shrinks holds an icon and no text, so there is no
glyph to measure it by. What there is, is the **clip rectangle** the renderer pushes around each
cell it paints an icon into: one `re W n` pair per cell, whose height is the row's own and whose
consecutive origins are the row pitch.

Compare against 26.2.4.2's own answer, which is not a rendering at all — `--convert-to fods`
writes `style:row-height` on the row style — so the two legs are independent.
"""
import re
import sys
import zlib

CLIP = re.compile(rb"([\d.]+) ([\d.]+) ([\d.]+) ([\d.]+) re\s+W\s+n")


def clips(pdf):
    data = open(pdf, "rb").read()
    out = []
    for found in re.finditer(rb"stream\r?\n(.*?)\r?\nendstream", data, re.S):
        body = found.group(1)
        try:
            body = zlib.decompress(body)
        except zlib.error:
            continue
        for box in CLIP.finditer(body):
            x, y, w, h = (float(v) for v in box.groups())
            out.append((round(y, 4), round(h, 4)))
    return out


for path in sys.argv[1:]:
    boxes = clips(path)
    if not boxes:
        print(f"{path}\tno clipped cells")
        continue
    ys = sorted({y for y, _ in boxes}, reverse=True)
    steps = [round(ys[i] - ys[i + 1], 4) for i in range(len(ys) - 1)]
    heights = sorted({h for _, h in boxes})
    print(f"{path}\t{len(boxes)} clipped cells\theights {heights}\tpitches {steps}")
