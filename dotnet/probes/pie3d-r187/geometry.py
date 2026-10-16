#!/usr/bin/env python3
"""Fit the 3-D pie's projected geometry out of the reference's own PDF.

    ./geometry.py <pdf> [...]

**26.2.4.2 rasterises a 3-D chart.** The pie is not vector geometry in the PDF at all: the
whole solid arrives as one `Do` of an RGB image with a soft mask, and only the labels and the
chart's frame are drawn as operators. So the geometry has to be read out of the bitmap's own
silhouette, and the placement matrix is what turns pixels into points.

For each file this prints, in points:

    A      the top face's horizontal semi-axis
    B      its vertical semi-axis
    B/A    the squash, which is what the elevation shows up as
    depth  the extruded side wall's projected height, below the ellipse's lower arc

The upper boundary of the silhouette is fitted as an ellipse by least squares over every
column; the fit's own residual is printed, because a projection with real perspective in it
would not be an ellipse and the residual is how you would know.
"""
import sys

import numpy as np
import pymupdf


def placement(document, page):
    """The `cm` matrix of each image `Do`, keyed by XObject name."""
    import re
    stream = b"".join(document.xref_stream(x) for x in page.get_contents()).decode("latin-1")
    found = {}
    for m in re.finditer(
            r"q\s+([\d.eE+-]+)\s+([\d.eE+-]+)\s+([\d.eE+-]+)\s+([\d.eE+-]+)"
            r"\s+([\d.eE+-]+)\s+([\d.eE+-]+)\s+cm\s*/(\w+)\s+Do", stream):
        found[m.group(7)] = tuple(float(m.group(i)) for i in (1, 4, 5, 6))
    return found


def fit(document, page, xref, name, place):
    pix = pymupdf.Pixmap(document, xref)
    mask = pymupdf.Pixmap(document, name) if name else None
    if mask is not None:
        opaque = np.frombuffer(mask.samples, dtype=np.uint8) \
                   .reshape(mask.height, mask.width, mask.n)[:, :, 0] > 128
    else:
        opaque = np.full((pix.height, pix.width), True)

    height, width = opaque.shape
    top = np.full(width, -1)
    bottom = np.full(width, -1)
    for x in range(width):
        rows = np.nonzero(opaque[:, x])[0]
        if len(rows):
            top[x] = rows.min()
            bottom[x] = rows.max()
    columns = np.nonzero(top >= 0)[0]
    if len(columns) < 20:
        return None

    left, right = columns.min(), columns.max()
    cx = (left + right) / 2.0
    a = (right - left) / 2.0
    u = (columns - cx) / a
    inner = np.abs(u) < 0.97
    root = np.sqrt(np.clip(1 - u[inner] ** 2, 0, 1))
    solution, *_ = np.linalg.lstsq(
        np.vstack([np.ones_like(root), -root]).T, top[columns][inner].astype(float), rcond=None)
    cy, b = solution
    residual = np.abs(cy - b * np.sqrt(np.clip(1 - u ** 2, 0, 1)) - top[columns])

    lower = cy + b * np.sqrt(np.clip(1 - u ** 2, 0, 1))
    wall = bottom[columns] - lower
    middle = np.abs(u) < 0.6

    pw, ph, px, py = place
    sx, sy = pw / width, ph / height
    return dict(A=a * sx, B=b * sy, depth=float(np.median(wall[middle])) * sy,
                fit_px=float(residual.mean()), worst_px=float(residual.max()))


def main() -> int:
    print("file\tA\tB\tB/A\tdepth\tdepth/2A\tfit_px\tworst_px")
    for path in sys.argv[1:]:
        document = pymupdf.open(path)
        page = document[0]
        where = placement(document, page)
        best = None
        for image in page.get_images(full=True):
            xref, smask, w, h = image[0], image[1], image[2], image[3]
            if w * h < 100000:
                continue
            place = where.get(image[7])
            if place is None:
                continue
            got = fit(document, page, xref, smask, place)
            if got and (best is None or got["A"] > best["A"]):
                best = got
        stem = path.rsplit("/", 1)[-1]
        if best is None:
            print("%s\t-\t-\t-\t-\t-\t-\t-" % stem)
            continue
        print("%s\t%.2f\t%.2f\t%.4f\t%.2f\t%.4f\t%.2f\t%.1f"
              % (stem, best["A"], best["B"], best["B"] / best["A"], best["depth"],
                 best["depth"] / (2 * best["A"]), best["fit_px"], best["worst_px"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
