#!/usr/bin/env python3
"""Compare two PDFs as images, and say where and how their pages differ.

    pdf-image-diff.py <ours.pdf> <ref.pdf> [options]

Renders both to PNG at a modest resolution, diffs them page by page, groups the
differing pixels into regions, and reports each region's position on the page with a
guess at what kind of difference it is — missing ink, extra ink, a shading or fill, a
rule, a colour shift.

Writes into --outdir:

    ours/page-NNN.png    what we produced
    ref/page-NNN.png     what the reference produced
    diff/page-NNN.png    the reference in grey with differing regions boxed in red

Exit status is 0 when no page has a major difference, 1 when one does, and 2 when the
comparison could not be made at all.

WHEN TO USE THIS
────────────────
**Only after page count and word count already agree.** This tool answers "the right
text is on the right page, but does the page *look* right" — and it cannot answer
anything else. If pagination differs, page 3 here is compared against a different page 3
there and every region it reports is noise. The script enforces this: it refuses to run
when the two PDFs have different page counts.

It is deliberately a coarse instrument. At 512 pixels on the longest edge a page is
about 1/6 of its rendered size, so a half-point line-break difference vanishes and a
missing background panel does not. That is the point: it finds the differences that
survive being squinted at, which are the ones worth a person's attention.

WHY 512
───────
Big enough that a missing logo, an absent fill, a wrong-coloured table header and a
displaced block are all several regions across. Small enough that a page diffs in about
a tenth of a second in pure Python and a whole corpus is practical. Text at this scale
is a texture rather than glyphs, which is why the hints talk about "ink" instead of
pretending to read it.

No third-party imaging library: poppler renders to PPM, which is a header and raw RGB,
and PNG is zlib plus four chunks. Adding numpy or Pillow to read two rectangles of bytes
would be a dependency for its own sake.
"""
import argparse
import pathlib
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib
from collections import deque

# A channel must differ by more than this for a pixel to count. Antialiasing along the
# same glyph edge routinely differs by 20-30 between two renderers that agree about
# where the glyph is, and counting that would drown everything else.
DEFAULT_THRESHOLD = 40

# How far apart two differing pixels can be and still be called one region. Three pixels
# at 512 joins the strokes of a word without joining two columns of a table.
DILATE = 3

# A region smaller than this fraction of the page is not worth reporting. 0.04% of a
# 512x512 page is about 100 pixels — a character or two.
DEFAULT_MIN_AREA = 0.0004

# A page is "majorly different" when this fraction of it differs, or when any single
# region exceeds MAJOR_REGION.
MAJOR_PAGE = 0.01
MAJOR_REGION = 0.004


def run(cmd: list[str]) -> None:
    proc = subprocess.run(cmd, capture_output=True)
    if proc.returncode != 0:
        raise RuntimeError(f"{cmd[0]} failed: {proc.stderr.decode('utf-8', 'replace')[:400]}")


def page_count(pdf: pathlib.Path) -> int:
    out = subprocess.run(["pdfinfo", str(pdf)], capture_output=True).stdout.decode("utf-8", "replace")
    m = re.search(r"^Pages:\s+(\d+)", out, re.M)
    return int(m.group(1)) if m else -1


def render(pdf: pathlib.Path, into: pathlib.Path, long_edge: int) -> list[pathlib.Path]:
    """Every page as a PPM, longest edge scaled to `long_edge`."""
    into.mkdir(parents=True, exist_ok=True)
    run(["pdftoppm", "-scale-to", str(long_edge), str(pdf), str(into / "p")])
    return sorted(into.glob("p-*.ppm"))


def read_ppm(path: pathlib.Path) -> tuple[int, int, bytes]:
    """Width, height and raw RGB. P6 only, which is what pdftoppm writes."""
    data = path.read_bytes()
    fields, pos = [], 2
    while len(fields) < 3:
        while pos < len(data) and data[pos:pos + 1].isspace():
            pos += 1
        if data[pos:pos + 1] == b"#":                     # comments are legal in the header
            while data[pos:pos + 1] not in (b"\n", b""):
                pos += 1
            continue
        start = pos
        while pos < len(data) and not data[pos:pos + 1].isspace():
            pos += 1
        fields.append(int(data[start:pos]))
    return fields[0], fields[1], data[pos + 1:]


def write_png(path: pathlib.Path, width: int, height: int, rgb: bytes) -> None:
    """Truecolour 8-bit PNG. Filter type 0 on every row; zlib does the rest."""
    raw = bytearray()
    stride = width * 3
    for y in range(height):
        raw.append(0)
        raw += rgb[y * stride:(y + 1) * stride]

    def chunk(tag: bytes, body: bytes) -> bytes:
        return (struct.pack(">I", len(body)) + tag + body
                + struct.pack(">I", zlib.crc32(tag + body) & 0xFFFFFFFF))

    path.write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(bytes(raw), 6))
        + chunk(b"IEND", b""))


def difference_mask(a: bytes, b: bytes, count: int, threshold: int) -> bytearray:
    """One byte per pixel: 1 where any channel differs by more than the threshold."""
    mask = bytearray(count)
    for i in range(count):
        j = i * 3
        if (abs(a[j] - b[j]) > threshold
                or abs(a[j + 1] - b[j + 1]) > threshold
                or abs(a[j + 2] - b[j + 2]) > threshold):
            mask[i] = 1
    return mask


def dilate(mask: bytearray, w: int, h: int, radius: int) -> bytearray:
    """Box dilation via prefix sums — two O(n) passes rather than a per-pixel window."""
    wide = bytearray(w * h)
    for y in range(h):
        row = y * w
        run_len = 0
        for x in range(w):                                # leftward reach
            run_len = radius + 1 if mask[row + x] else max(run_len - 1, 0)
            if run_len:
                wide[row + x] = 1
        run_len = 0
        for x in range(w - 1, -1, -1):                    # rightward reach
            run_len = radius + 1 if mask[row + x] else max(run_len - 1, 0)
            if run_len:
                wide[row + x] = 1

    out = bytearray(w * h)
    for x in range(w):
        run_len = 0
        for y in range(h):
            run_len = radius + 1 if wide[y * w + x] else max(run_len - 1, 0)
            if run_len:
                out[y * w + x] = 1
        run_len = 0
        for y in range(h - 1, -1, -1):
            run_len = radius + 1 if wide[y * w + x] else max(run_len - 1, 0)
            if run_len:
                out[y * w + x] = 1
    return out


def regions(mask: bytearray, w: int, h: int, min_pixels: int) -> list[dict]:
    """Connected components of the dilated mask, as bounding boxes."""
    seen = bytearray(w * h)
    found = []
    for start in range(w * h):
        if not mask[start] or seen[start]:
            continue
        queue = deque([start])
        seen[start] = 1
        x0 = x1 = start % w
        y0 = y1 = start // w
        pixels = 0
        while queue:
            p = queue.popleft()
            pixels += 1
            px, py = p % w, p // w
            if px < x0: x0 = px
            if px > x1: x1 = px
            if py < y0: y0 = py
            if py > y1: y1 = py
            for q in ((p - 1 if px else -1), (p + 1 if px + 1 < w else -1),
                      (p - w if py else -1), (p + w if py + 1 < h else -1)):
                if q >= 0 and mask[q] and not seen[q]:
                    seen[q] = 1
                    queue.append(q)
        if pixels >= min_pixels:
            found.append({"x0": x0, "y0": y0, "x1": x1, "y1": y1, "pixels": pixels})
    found.sort(key=lambda r: -r["pixels"])
    return found


def where(r: dict, w: int, h: int) -> str:
    """A human's description of the region's place on the page."""
    cx = (r["x0"] + r["x1"]) / 2 / w
    cy = (r["y0"] + r["y1"]) / 2 / h
    row = "top" if cy < 0.33 else "middle" if cy < 0.67 else "bottom"
    col = "left" if cx < 0.33 else "centre" if cx < 0.67 else "right"
    if (r["x1"] - r["x0"]) > 0.8 * w and (r["y1"] - r["y0"]) > 0.8 * h:
        return "the whole page"
    return f"{row}-{col}"


def classify(r: dict, ours: bytes, ref: bytes, w: int) -> str:
    """What kind of difference this region most likely is.

    Deliberately a small number of coarse buckets. The hint is there to tell someone
    which of several very different investigations to start, not to be right about the
    cause — a wrong guess that sends you to the right part of the page still saves the
    search.
    """
    ow = r["x1"] - r["x0"] + 1
    oh = r["y1"] - r["y0"] + 1
    our_sum = ref_sum = 0
    our_chroma = ref_chroma = 0
    n = 0
    step = max(1, (ow * oh) // 4000)                      # sample big regions rather than walk them
    for y in range(r["y0"], r["y1"] + 1):
        for x in range(r["x0"], r["x1"] + 1, step):
            j = (y * w + x) * 3
            o0, o1, o2 = ours[j], ours[j + 1], ours[j + 2]
            f0, f1, f2 = ref[j], ref[j + 1], ref[j + 2]
            our_sum += (o0 + o1 + o2) // 3
            ref_sum += (f0 + f1 + f2) // 3
            our_chroma += max(o0, o1, o2) - min(o0, o1, o2)
            ref_chroma += max(f0, f1, f2) - min(f0, f1, f2)
            n += 1
    if not n:
        return "unclassified"

    our_luma, ref_luma = our_sum / n, ref_sum / n
    fill = r["pixels"] / (ow * oh)
    thin = min(ow, oh) <= 4 and max(ow, oh) >= 8 * max(1, min(ow, oh))
    chroma_gap = abs(our_chroma - ref_chroma) / n

    if thin:
        return "a rule or border" + (" missing here" if our_luma > ref_luma + 20 else "")
    # A large, solidly-filled region is an area of colour rather than a cluster of marks.
    if fill > 0.85 and ow * oh > 400:
        if our_luma > ref_luma + 20:
            return "a fill or background shading the reference has and we do not"
        if ref_luma > our_luma + 20:
            return "a fill or background shading we draw and the reference does not"
        if chroma_gap > 12:
            return "a fill of the wrong colour"
        return "a solid area drawn differently"
    if our_luma > ref_luma + 25:
        return "ink missing from ours — a graphic, glyphs or a fill"
    if ref_luma > our_luma + 25:
        return "ink we draw that the reference does not"
    if chroma_gap > 12:
        return "the same marks in a different colour"
    return "marks displaced or reshaped"


def annotate(ours: bytes, ref: bytes, w: int, h: int, found: list[dict]) -> bytes:
    """The reference, faded, with each region boxed in red and its interior tinted."""
    out = bytearray(w * h * 3)
    for i in range(w * h):
        j = i * 3
        grey = (ref[j] + ref[j + 1] + ref[j + 2]) // 3
        pale = 255 - (255 - grey) // 3                     # keep it legible under the marks
        out[j] = out[j + 1] = out[j + 2] = pale
    for r in found:
        for x in range(r["x0"], r["x1"] + 1):
            for y in (r["y0"], r["y1"]):
                j = (y * w + x) * 3
                out[j], out[j + 1], out[j + 2] = 220, 0, 0
        for y in range(r["y0"], r["y1"] + 1):
            for x in (r["x0"], r["x1"]):
                j = (y * w + x) * 3
                out[j], out[j + 1], out[j + 2] = 220, 0, 0
        for y in range(r["y0"] + 1, r["y1"]):
            for x in range(r["x0"] + 1, r["x1"]):
                j = (y * w + x) * 3
                out[j] = min(255, out[j] + 40)
                out[j + 1] = max(0, out[j + 1] - 25)
                out[j + 2] = max(0, out[j + 2] - 25)
    return bytes(out)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("ours")
    ap.add_argument("ref")
    ap.add_argument("--outdir", default=None)
    ap.add_argument("--long-edge", type=int, default=512)
    ap.add_argument("--threshold", type=int, default=DEFAULT_THRESHOLD)
    ap.add_argument("--min-area", type=float, default=DEFAULT_MIN_AREA)
    ap.add_argument("--quiet", action="store_true", help="report only pages with major differences")
    args = ap.parse_args(argv[1:])

    ours_pdf, ref_pdf = pathlib.Path(args.ours), pathlib.Path(args.ref)
    for p in (ours_pdf, ref_pdf):
        if not p.is_file():
            print(f"no such file: {p}", file=sys.stderr)
            return 2

    # The gate. Comparing page n of one against page n of the other is meaningless when
    # the two paginate differently, and every region reported would be an artefact of
    # the misalignment rather than a rendering difference.
    a, b = page_count(ours_pdf), page_count(ref_pdf)
    if a != b or a < 0:
        print(f"page counts differ ({a} vs {b}) — fix pagination before comparing images",
              file=sys.stderr)
        return 2

    outdir = pathlib.Path(args.outdir) if args.outdir else pathlib.Path(tempfile.mkdtemp())
    (outdir / "ours").mkdir(parents=True, exist_ok=True)
    (outdir / "ref").mkdir(parents=True, exist_ok=True)
    (outdir / "diff").mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as tmp:
        tmp = pathlib.Path(tmp)
        ours_pages = render(ours_pdf, tmp / "o", args.long_edge)
        ref_pages = render(ref_pdf, tmp / "r", args.long_edge)

        bad = 0
        print("page\tdiff%\tregions\tverdict")
        for i, (op, rp) in enumerate(zip(ours_pages, ref_pages), start=1):
            ow, oh, orgb = read_ppm(op)
            rw, rh, rrgb = read_ppm(rp)
            write_png(outdir / "ours" / f"page-{i:03d}.png", ow, oh, orgb)
            write_png(outdir / "ref" / f"page-{i:03d}.png", rw, rh, rrgb)

            if (ow, oh) != (rw, rh):
                # Same page count but a different paper size or orientation. Real, and
                # not something a pixel diff can say anything useful about.
                print(f"{i}\t-\t-\tpage size differs: {ow}x{oh} vs {rw}x{rh}")
                bad += 1
                continue

            count = ow * oh
            mask = difference_mask(orgb, rrgb, count, args.threshold)
            raw_diff = sum(mask) / count
            found = regions(dilate(mask, ow, oh, DILATE), ow, oh,
                            max(8, int(args.min_area * count)))
            write_png(outdir / "diff" / f"page-{i:03d}.png", ow, oh,
                      annotate(orgb, rrgb, ow, oh, found))

            biggest = max((r["pixels"] / count for r in found), default=0.0)
            major = raw_diff > MAJOR_PAGE or biggest > MAJOR_REGION
            if major:
                bad += 1
            elif args.quiet:
                continue

            print(f"{i}\t{raw_diff * 100:.2f}\t{len(found)}\t{'MAJOR' if major else 'ok'}")
            for r in found[:6] if major else []:
                print(f"\t\t\t{where(r, ow, oh)}: {classify(r, orgb, rrgb, ow)} "
                      f"({r['pixels'] * 100 / count:.2f}% of page, "
                      f"x {r['x0'] / ow:.2f}-{r['x1'] / ow:.2f}, "
                      f"y {r['y0'] / oh:.2f}-{r['y1'] / oh:.2f})")

    print(f"\n{a} pages, {bad} with major differences")
    print(f"images in {outdir}")
    return 1 if bad else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv))
    except RuntimeError as exc:
        print(exc, file=sys.stderr)
        raise SystemExit(2)
