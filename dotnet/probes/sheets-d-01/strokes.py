#!/usr/bin/env python3
"""Census every *line* a PDF page paints, recording the idiom that painted it.

    strokes.py <pdf> [--page N] [--all]

WHY ITS OWN SCRIPT, rather than `pdf-ops.py --only stroke`
──────────────────────────────────────────────────────────
The question this round asks is "how many strokes lie on one grid line, and do they overlap".
`pdf-ops.py` normalises a stroke into a box and pairs it against another render's; that hides
exactly the two things needed here — whether the run came out as one `m/l` pair or five, and
whether consecutive segments share an endpoint or overlap it.

It also has to see *both* renderers' idioms. LibreOffice writes a border as `m … l S` under a
`w` width; X-Ray.Office writes the same thing; but a *fill* rectangle is `re f` on both, and
a hairline drawn as a filled rectangle (`re f` with one dimension near zero) is ink on a grid
line that a stroke-only census scores as zero. All three are collected and labelled, so a count
is never quoted without saying which idiom produced it.

Emits one TSV row per axis-aligned line-like mark:
    page  kind  orient  at  from  to  width  colour  idiom
where kind is stroke|fillrect, idiom is `ml` (moveto/lineto), `re` (rectangle) or `poly`.
"""
import argparse, math, pathlib, re, sys, zlib

sys.path.insert(0, "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts")
from importlib import import_module
_ops = import_module("pdf-ops") if False else None
import importlib.util
spec = importlib.util.spec_from_file_location(
    "pdfops",
    "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py")
pdfops = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pdfops)

tokenise, multiply, apply_m, colour_hex = (
    pdfops.tokenise, pdfops.multiply, pdfops.apply, pdfops.colour_hex)
Objects = pdfops.Objects

FLAT = 0.35   # a mark thinner than this in one axis is a line, not a box


def page_streams(pdf):
    blob = pathlib.Path(pdf).read_bytes()
    objects = Objects(blob)
    out = []
    for body in objects.pages():
        m = re.search(rb"/Contents\s*(\[[^\]]*\]|\d+\s+\d+\s+R)", body)
        data = b""
        if m:
            for num in re.findall(rb"(\d+)\s+\d+\s+R", m.group(1)):
                data += objects.stream_of(objects.raw(int(num))) + b"\n"
        out.append(data)
    return out


def interpret(stream, page):
    """Yield line-like marks. Colour is taken from the operator that actually paints."""
    ctm = [1, 0, 0, 1, 0, 0]
    stack = []
    width = 1.0
    stroke_col = [0.0]
    fill_col = [0.0]
    operands = []
    subpaths = []      # list of (idiom, [pts in user space])
    cur = None
    rows = []

    def emit(idiom, pts, kind, colour):
        # a subpath of two points -> one segment; a rect -> four edges
        for i in range(len(pts) - 1):
            (x0, y0), (x1, y1) = pts[i], pts[i + 1]
            if abs(x0 - x1) < 1e-6 and abs(y0 - y1) < 1e-6:
                continue
            if abs(y0 - y1) < 1e-6:
                rows.append((page, kind, "H", round(y0, 3), round(min(x0, x1), 3),
                             round(max(x0, x1), 3), round(w_dev, 4), colour, idiom))
            elif abs(x0 - x1) < 1e-6:
                rows.append((page, kind, "V", round(x0, 3), round(min(y0, y1), 3),
                             round(max(y0, y1), 3), round(w_dev, 4), colour, idiom))

    w_dev = 1.0
    for kind, value in tokenise(stream):
        if kind in ("num",):
            operands.append(value); continue
        if kind != "op":
            operands.append(value); continue
        op = value
        n = operands
        try:
            if op == "q":
                stack.append((ctm[:], width, stroke_col[:], fill_col[:]))
            elif op == "Q":
                if stack:
                    ctm, width, stroke_col, fill_col = stack.pop()
                    ctm = ctm[:]
            elif op == "cm" and len(n) >= 6:
                ctm = multiply([float(v) for v in n[-6:]], ctm)
            elif op == "w" and n:
                width = float(n[-1])
            elif op in ("RG", "rg", "K", "k", "G", "g", "SC", "sc", "SCN", "scn"):
                comps = [float(v) for v in n if isinstance(v, float)]
                col = colour_hex(comps)
                if op in ("RG", "K", "G", "SC", "SCN"):
                    stroke_col = [col]
                else:
                    fill_col = [col]
            elif op == "m" and len(n) >= 2:
                cur = ("ml", [apply_m(ctm, float(n[-2]), float(n[-1]))])
                subpaths.append(cur)
            elif op in ("l",) and len(n) >= 2 and cur is not None:
                cur[1].append(apply_m(ctm, float(n[-2]), float(n[-1])))
            elif op in ("c", "v", "y") and cur is not None:
                pts = [float(v) for v in n if isinstance(v, float)]
                for i in range(0, len(pts) - 1, 2):
                    cur[1].append(apply_m(ctm, pts[i], pts[i + 1]))
                cur = ("poly", cur[1])
                subpaths[-1] = cur
            elif op == "re" and len(n) >= 4:
                x, y, w, h = (float(v) for v in n[-4:])
                pts = [apply_m(ctm, x, y), apply_m(ctm, x + w, y),
                       apply_m(ctm, x + w, y + h), apply_m(ctm, x, y + h),
                       apply_m(ctm, x, y)]
                cur = ("re", pts)
                subpaths.append(cur)
            elif op == "h" and cur is not None and len(cur[1]) > 1:
                cur[1].append(cur[1][0])
            elif op in ("S", "s", "f", "F", "f*", "B", "B*", "b", "b*", "n"):
                sx = math.hypot(ctm[0], ctm[1]) or 1.0
                sy = math.hypot(ctm[2], ctm[3]) or 1.0
                w_dev = width * math.sqrt(abs(sx * sy))
                if op in ("s", "b", "b*") and cur is not None and len(cur[1]) > 1:
                    cur[1].append(cur[1][0])
                if op in ("S", "s", "B", "B*", "b", "b*"):
                    for idiom, pts in subpaths:
                        emit(idiom, pts, "stroke", stroke_col[0])
                if op in ("f", "F", "f*", "B", "B*", "b", "b*"):
                    for idiom, pts in subpaths:
                        xs = [p[0] for p in pts]; ys = [p[1] for p in pts]
                        bw, bh = max(xs) - min(xs), max(ys) - min(ys)
                        if bh < FLAT and bw >= bh:
                            w_dev = bh
                            rows.append((page, "fillrect", "H", round((min(ys)+max(ys))/2, 3),
                                         round(min(xs), 3), round(max(xs), 3),
                                         round(bh, 4), fill_col[0], idiom))
                        elif bw < FLAT and bh > bw:
                            w_dev = bw
                            rows.append((page, "fillrect", "V", round((min(xs)+max(xs))/2, 3),
                                         round(min(ys), 3), round(max(ys), 3),
                                         round(bw, 4), fill_col[0], idiom))
                subpaths = []; cur = None
        finally:
            if op not in ("m", "l", "c", "v", "y", "re", "h"):
                operands = []
            else:
                operands = []
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("pdf")
    ap.add_argument("--page", type=int)
    ap.add_argument("--tsv", action="store_true")
    a = ap.parse_args()
    streams = page_streams(a.pdf)
    print("page\tkind\torient\tat\tfrom\tto\twidth\tcolour\tidiom")
    for i, s in enumerate(streams, 1):
        if a.page and i != a.page:
            continue
        for r in interpret(s, i):
            print("\t".join(str(v) for v in r))


if __name__ == "__main__":
    main()
