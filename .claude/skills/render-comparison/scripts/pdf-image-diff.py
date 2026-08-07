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

# What makes a page "major" is an *ink imbalance*, not a difference.
#
# This distinction is the whole design. Two renderers that agree about a page still differ
# on almost every glyph pixel, because a two-pixel drift down the page makes every line of
# text land somewhere slightly different — measured on a plain one-page letter that matches
# the reference word for word, 9% of the page differs and every paragraph is a region. If
# "differs" meant "wrong", every document in the corpus would be wrong.
#
# So a region only counts when one side has substantially more ink in it than the other.
# Missing fills, absent graphics, undrawn rules and unreadable text all shift ink; a
# reflowed paragraph moves the same ink somewhere else and is reported as `shifted`.
INK_GAP = 22            # mean luma difference marking a region as present-vs-absent
MAJOR_REGION = 0.004    # ... and how much of the page such a region must cover
MAJOR_PAGE_INK = 0.012  # or this much of a page's total ink unaccounted for either way

# How far two renderings of the same page may differ in pixel size before the comparison is
# refused. `pdftoppm -scale-to` pins the long edge and rounds the short one, so a page size
# differing in the second decimal of a point lands a pixel apart. Two pixels and one percent
# are both well under the DILATE=3 region tolerance; beyond that it is a real paper-size
# difference and cropping would be hiding it.
SIZE_SLACK_PIXELS = 2
SIZE_SLACK_RATIO = 0.01


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


def crop(rgb: bytes, width: int, height: int, to_width: int, to_height: int) -> bytes:
    """The top-left `to_width` x `to_height` of an RGB buffer."""
    if (width, height) == (to_width, to_height):
        return rgb
    out = bytearray(to_width * to_height * 3)
    for y in range(to_height):
        src = y * width * 3
        dst = y * to_width * 3
        out[dst:dst + to_width * 3] = rgb[src:src + to_width * 3]
    return bytes(out)


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


def measure(r: dict, ours: bytes, ref: bytes, w: int) -> None:
    """Fill in a region's ink balance and colour spread, in place.

    `luma_gap` is the signed mean brightness difference over the region: positive when
    ours is lighter, which means the reference has ink here that we do not. It is what
    separates a missing thing from a moved thing, and therefore what the verdict rests on.
    """
    ow = r["x1"] - r["x0"] + 1
    oh = r["y1"] - r["y0"] + 1
    our_sum = ref_sum = our_chroma = ref_chroma = n = 0
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
    n = n or 1
    r["our_luma"] = our_sum / n
    r["ref_luma"] = ref_sum / n
    r["luma_gap"] = r["our_luma"] - r["ref_luma"]
    r["chroma_gap"] = abs(our_chroma - ref_chroma) / n
    r["fill"] = r["pixels"] / (ow * oh)
    r["thin"] = min(ow, oh) <= 4 and max(ow, oh) >= 8 * max(1, min(ow, oh))


def classify(r: dict) -> str:
    """What kind of difference this region most likely is.

    Deliberately a small number of coarse buckets. The hint is there to tell someone
    which of several very different investigations to start, not to be right about the
    cause — a wrong guess that sends you to the right part of the page still saves the
    search.
    """
    gap, chroma = r["luma_gap"], r["chroma_gap"]
    if r["thin"]:
        return ("a rule or border missing here" if gap > INK_GAP else
                "a rule or border we draw and the reference does not" if gap < -INK_GAP else
                "a rule or border drawn differently")
    # A large, solidly-filled region is an area of colour rather than a cluster of marks.
    if r["fill"] > 0.85 and (r["x1"] - r["x0"] + 1) * (r["y1"] - r["y0"] + 1) > 400:
        if gap > INK_GAP:
            return "a fill or background shading the reference has and we do not"
        if gap < -INK_GAP:
            return "a fill or background shading we draw and the reference does not"
        if chroma > 12:
            return "a fill of the wrong colour"
        return "a solid area drawn differently"
    if gap > INK_GAP:
        return "ink missing from ours — a graphic, glyphs or a fill"
    if gap < -INK_GAP:
        return "ink we draw that the reference does not"
    if chroma > 12:
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
        trimmed = 0
        print("page\tdiff%\tink%\t|ink|%\tregions\tverdict")
        for i, (op, rp) in enumerate(zip(ours_pages, ref_pages), start=1):
            ow, oh, orgb = read_ppm(op)
            rw, rh, rrgb = read_ppm(rp)
            write_png(outdir / "ours" / f"page-{i:03d}.png", ow, oh, orgb)
            write_png(outdir / "ref" / f"page-{i:03d}.png", rw, rh, rrgb)

            if (ow, oh) != (rw, rh):
                # `pdftoppm -scale-to` pins the longest edge and rounds the other from the
                # aspect ratio, so two pages differing by a hair -- 841.89 pt against 842.0 --
                # land on 288 and 289 and every page of the document was being skipped.
                # Measured: nine of the slides track's 163 documents were unmeasurable for
                # this reason alone, which made any change to them invisible.
                #
                # Crop to the common size rather than rescale. Both images are anchored at the
                # top-left and are within a rounding step of the same scale, so the worst
                # drift is one pixel at the far edge -- inside the DILATE=3 tolerance the
                # regions already carry. Rescaling would resample every pixel to remove a
                # difference smaller than the thing being measured.
                slack = max(abs(ow - rw), abs(oh - rh))
                relative = slack / max(ow, oh, rw, rh)
                if slack <= SIZE_SLACK_PIXELS and relative <= SIZE_SLACK_RATIO:
                    cw, ch = min(ow, rw), min(oh, rh)
                    orgb = crop(orgb, ow, oh, cw, ch)
                    rrgb = crop(rrgb, rw, rh, cw, ch)
                    ow, oh = cw, ch
                    trimmed += 1
                else:
                    # A genuinely different paper size or orientation. Real, and not something
                    # a pixel diff can say anything useful about.
                    print(f"{i}\t-\t-\tpage size differs: {ow}x{oh} vs {rw}x{rh}")
                    bad += 1
                    continue

            count = ow * oh
            mask = difference_mask(orgb, rrgb, count, args.threshold)
            raw_diff = sum(mask) / count
            found = regions(dilate(mask, ow, oh, DILATE), ow, oh,
                            max(8, int(args.min_area * count)))
            for r in found:
                measure(r, orgb, rrgb, ow)
            write_png(outdir / "diff" / f"page-{i:03d}.png", ow, oh,
                      annotate(orgb, rrgb, ow, oh, found))

            # Two ink figures, because one number cannot do both jobs.
            #
            # `ink%` is the *signed* sum: a region where we draw more cancels one where we draw
            # less. That is deliberate and is what makes it ignore reflow — moving ink from one
            # place to another nets to nought, while a missing fill does not. It is the right
            # verdict metric.
            #
            # But a signed sum is the wrong thing to *drive work down*, and that had to be
            # learned: a round that recoded symbol-font bullets made 24 documents embed the
            # reference's exact face count, moved a bullet's drawn width from 6.30 pt to 13.73
            # against the reference's 13.72 with zero differing pixels in the band, lowered raw
            # pixel difference on every document examined — and raised `ink%` by 0.91, because
            # it had been filling a deficit that was cancelling a surplus elsewhere. A metric
            # where a real improvement reads as a regression will send the next agent backwards.
            #
            # `|ink|%` sums the same regions unsigned. It cannot tell reflow from loss, so it is
            # no good as a verdict — but it never rewards leaving a defect in place, so it is
            # the column to rank by and to compare across a round.
            areas = [(r["luma_gap"], (r["x1"] - r["x0"] + 1) * (r["y1"] - r["y0"] + 1))
                     for r in found]
            page_ink = abs(sum(gap * area for gap, area in areas)) / (count * 255)
            page_ink_abs = sum(abs(gap) * area for gap, area in areas) / (count * 255)

            heavy = [r for r in found
                     if abs(r["luma_gap"]) > INK_GAP and r["pixels"] / count > MAJOR_REGION]
            major = bool(heavy) or page_ink > MAJOR_PAGE_INK
            if major:
                bad += 1
            elif args.quiet:
                continue

            verdict = "MAJOR" if major else ("shifted" if raw_diff > 0.02 else "ok")
            print(f"{i}\t{raw_diff * 100:.2f}\t{page_ink * 100:.2f}"
                  f"\t{page_ink_abs * 100:.2f}\t{len(found)}\t{verdict}")
            # Ink-imbalanced regions first: they are the ones that mean something is
            # missing rather than moved.
            for r in (sorted(found, key=lambda r: -abs(r["luma_gap"]) * r["pixels"])[:6]
                      if major else []):
                print(f"\t\t\t\t{where(r, ow, oh)}: {classify(r)} "
                      f"({r['pixels'] * 100 / count:.2f}% of page, "
                      f"x {r['x0'] / ow:.2f}-{r['x1'] / ow:.2f}, "
                      f"y {r['y0'] / oh:.2f}-{r['y1'] / oh:.2f})")

    print(f"\n{a} pages, {bad} with major differences")
    if trimmed:
        # Say it rather than silently cropping. A reader who sees a near-zero ink figure
        # deserves to know the two images were not quite the same shape.
        print(f"{trimmed} page(s) cropped to a common size, within "
              f"{SIZE_SLACK_PIXELS} px — rounding in pdftoppm's aspect, not a paper difference")
    print(f"images in {outdir}")
    return 1 if bad else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv))
    except RuntimeError as exc:
        print(exc, file=sys.stderr)
        raise SystemExit(2)
