#!/usr/bin/env python3
"""Every clip rectangle on a page, in page coordinates.

Written for the re-check of §2.1, where the first attempt read the `re` operands raw and got
the right answer on five documents and nonsense on two.  `012_`/`013_Contextures` state their
block clip inside a `cm`, so the operands are in the chart's space and not the page's; the
same rectangle that reads `54 401.046 129.883 318.954` in the reference's stream reads
`0.883 530.961 …` in ours.  Anything that compares two renderings' clips has to carry the
graphics-state stack and the current transformation matrix, which is what this does.

    clip-rects.py FILE.pdf PAGE          -> the clips, deduplicated, page clip marked
    clip-rects.py FILE.pdf PAGE --paths  -> instead, drawn paths wider than 300 pt

The `--paths` mode is how §2.1's plot rectangle was found: a chart's plot area is not a clip
at all on these pages, it is the wall's fill and the gridlines drawn across it, and the seven
of them agree on one x extent.
"""
import re
import sys

import pymupdf

TOKEN = re.compile(
    rb'(?s)(\[[^\]]*\]|\((?:\\.|[^\\)])*\)|<[^>]*>|/[^\s/\[\]<>()]+|[-+.\d]+|[A-Za-z\'"*]+)')


def _mul(a, b):
    return (a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3],
            a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3],
            a[4] * b[0] + a[5] * b[2] + b[4], a[4] * b[1] + a[5] * b[3] + b[5])


def clips(path, pageno):
    """Every `re … W n` / `re … W* n` on the page, as (x0, y0, x1, y1) in page coordinates."""
    doc = pymupdf.open(path)
    page = doc[pageno - 1]
    stream, size = page.read_contents(), (page.rect.width, page.rect.height)
    doc.close()

    ctm, saved, operands, last_rect, out = (1, 0, 0, 1, 0, 0), [], [], None, []
    for match in TOKEN.finditer(stream):
        token = match.group(1).decode('latin-1')
        if re.fullmatch(r'[-+.\d]+', token):
            operands.append(float(token))
            continue
        if token == 'q':
            saved.append(ctm)
        elif token == 'Q':
            ctm = saved.pop() if saved else ctm
        elif token == 'cm' and len(operands) >= 6:
            ctm = _mul(tuple(operands[-6:]), ctm)
        elif token == 're' and len(operands) >= 4:
            last_rect = tuple(operands[-4:])
        elif token in ('W', 'W*') and last_rect:
            x, y, w, h = last_rect
            corners = [(x, y), (x + w, y), (x, y + h), (x + w, y + h)]
            moved = [(px * ctm[0] + py * ctm[2] + ctm[4], px * ctm[1] + py * ctm[3] + ctm[5])
                     for px, py in corners]
            xs, ys = [p[0] for p in moved], [p[1] for p in moved]
            out.append((round(min(xs), 3), round(min(ys), 3),
                        round(max(xs), 3), round(max(ys), 3)))
        operands = []
    return out, size


def wide_paths(path, pageno, floor=300.0):
    """Drawn paths wider than `floor`, deduplicated — the plot rectangle of §2.1."""
    doc = pymupdf.open(path)
    page = doc[pageno - 1]
    seen, out = set(), []
    for drawing in page.get_drawings():
        r = drawing['rect']
        key = (round(r.x0, 2), round(r.y0, 2), round(r.x1, 2), round(r.y1, 2))
        if key in seen or r.x1 - r.x0 <= floor:
            continue
        seen.add(key)
        out.append((key, drawing['type']))
    doc.close()
    return out


def main(argv):
    path, pageno = argv[1], int(argv[2])
    if '--paths' in argv:
        for key, kind in wide_paths(path, pageno):
            print(f"  {key} w={key[2] - key[0]:.2f} type={kind}")
        return
    found, (w, h) = clips(path, pageno)
    print(f"page {w:.2f} x {h:.2f}")
    for rect in dict.fromkeys(found):
        area = (rect[2] - rect[0]) * (rect[3] - rect[1])
        print(f"  {rect} w={rect[2] - rect[0]:.3f}{'   <- page clip' if area > 0.9 * w * h else ''}")


if __name__ == "__main__":
    main(sys.argv)
