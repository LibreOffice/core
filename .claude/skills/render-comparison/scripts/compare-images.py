#!/usr/bin/env python3
"""Compare rendered page images against LibreOffice reference images.

Reports several independent metrics rather than one score, because no single number
distinguishes "a shape is missing" from "everything shifted down by 3 pixels" -- and
those two need completely different fixes. See ../SKILL.md for how to read them.

Deliberately dependency-free: PNG is decoded with zlib and struct from the standard
library, so this runs in a bare container with no pip install.
"""

from __future__ import annotations

import argparse
import hashlib
import pathlib
import re
import struct
import sys
import zlib

TILE = 32              # tile edge, in pixels, for regional metrics
DIFF_TOLERANCE = 12    # per-channel 0-255 difference treated as "same" (antialiasing)
SHIFT_SEARCH = 40      # max vertical offset probed when detecting a shift


# --------------------------------------------------------------------------- PNG input

class Image:
    """A greyscale image. Colour is reduced to luminance on load."""

    __slots__ = ("width", "height", "gray")

    def __init__(self, width: int, height: int, gray: bytearray):
        self.width = width
        self.height = height
        self.gray = gray

    def at(self, x: int, y: int) -> int:
        return self.gray[y * self.width + x]


def _paeth(a: int, b: int, c: int) -> int:
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    return b if pb <= pc else c


def read_png(path: pathlib.Path) -> Image:
    """Decode a PNG to greyscale.

    Supports the subset that rasterisers actually emit: 8-bit greyscale, RGB, RGBA and
    palette, non-interlaced. Anything else raises, rather than silently mis-decoding
    and producing a plausible-looking wrong number.
    """
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path}: not a PNG")

    pos = 8
    width = height = bit_depth = colour_type = interlace = -1
    idat = bytearray()
    palette = b""
    while pos < len(data):
        (length,) = struct.unpack(">I", data[pos:pos + 4])
        ctype = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + length]
        pos += 12 + length          # 4 len + 4 type + body + 4 crc
        if ctype == b"IHDR":
            width, height, bit_depth, colour_type, _, _, interlace = struct.unpack(
                ">IIBBBBB", body)
        elif ctype == b"PLTE":
            palette = body
        elif ctype == b"IDAT":
            idat += body
        elif ctype == b"IEND":
            break

    if bit_depth != 8:
        raise ValueError(f"{path}: bit depth {bit_depth} unsupported (need 8)")
    if interlace != 0:
        raise ValueError(f"{path}: interlaced PNG unsupported")
    channels = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}.get(colour_type)
    if channels is None:
        raise ValueError(f"{path}: colour type {colour_type} unsupported")

    raw = zlib.decompress(bytes(idat))
    stride = width * channels
    gray = bytearray(width * height)
    prev = bytearray(stride)
    off = 0
    for y in range(height):
        filt = raw[off]
        line = bytearray(raw[off + 1:off + 1 + stride])
        off += 1 + stride
        # Undo the per-scanline filter (PNG spec section 9).
        if filt == 1:
            for i in range(channels, stride):
                line[i] = (line[i] + line[i - channels]) & 0xFF
        elif filt == 2:
            for i in range(stride):
                line[i] = (line[i] + prev[i]) & 0xFF
        elif filt == 3:
            for i in range(stride):
                left = line[i - channels] if i >= channels else 0
                line[i] = (line[i] + ((left + prev[i]) >> 1)) & 0xFF
        elif filt == 4:
            for i in range(stride):
                left = line[i - channels] if i >= channels else 0
                upleft = prev[i - channels] if i >= channels else 0
                line[i] = (line[i] + _paeth(left, prev[i], upleft)) & 0xFF
        elif filt != 0:
            raise ValueError(f"{path}: bad filter type {filt}")

        base = y * width
        if colour_type == 3:
            for x in range(width):
                p = line[x] * 3
                r, g, b = palette[p], palette[p + 1], palette[p + 2]
                gray[base + x] = (r * 299 + g * 587 + b * 114) // 1000
        elif channels == 1:
            gray[base:base + width] = line
        elif channels == 2:                      # grey + alpha, composite onto white
            for x in range(width):
                v, a = line[x * 2], line[x * 2 + 1]
                gray[base + x] = (v * a + 255 * (255 - a)) // 255
        else:
            for x in range(width):
                i = x * channels
                r, g, b = line[i], line[i + 1], line[i + 2]
                v = (r * 299 + g * 587 + b * 114) // 1000
                if channels == 4:                # composite onto white
                    a = line[i + 3]
                    v = (v * a + 255 * (255 - a)) // 255
                gray[base + x] = v
        prev = line

    return Image(width, height, gray)


# ----------------------------------------------------------------------------- metrics

def ink_fraction(img: Image) -> float:
    """Fraction of pixels that are not near-white, i.e. how much was actually drawn."""
    return sum(1 for v in img.gray if v < 250) / max(1, len(img.gray))


def row_ink_profile(img: Image) -> list[int]:
    return [sum(1 for x in range(img.width) if img.at(x, y) < 250)
            for y in range(img.height)]


def best_vertical_shift(a: Image, b: Image) -> tuple[int, float]:
    """Find the vertical offset that best aligns two ink-per-row profiles.

    A non-zero result is strong evidence of a reflow: the content is present but has
    moved bodily down or up the page.
    """
    pa, pb = row_ink_profile(a), row_ink_profile(b)
    n = min(len(pa), len(pb))
    if n == 0:
        return 0, 0.0
    best_shift, best_cost = 0, None
    limit = min(SHIFT_SEARCH, max(1, n // 4))
    for shift in range(-limit, limit + 1):
        cost = count = 0
        for y in range(n):
            sy = y + shift
            if 0 <= sy < n:
                cost += abs(pa[y] - pb[sy])
                count += 1
        if count:
            norm = cost / count
            if best_cost is None or norm < best_cost:
                best_shift, best_cost = shift, norm
    return best_shift, best_cost or 0.0


def tile_metrics(a: Image, b: Image) -> tuple[float, int, int]:
    """Return (worst tile mean error, tiles differing, tiles that merely moved).

    A tile counts as "shifted" when it matches the reference poorly in place but well
    at some small offset -- the signature of reflowed rather than wrong content.
    """
    worst = 0.0
    differing = shifted = 0
    for ty in range(0, a.height, TILE):
        for tx in range(0, a.width, TILE):
            h = min(TILE, a.height - ty)
            w = min(TILE, a.width - tx)
            total = 0
            for y in range(ty, ty + h):
                ra = y * a.width
                rb = y * b.width
                for x in range(tx, tx + w):
                    total += abs(a.gray[ra + x] - b.gray[rb + x])
            mean = total / (w * h) / 255.0
            worst = max(worst, mean)
            if mean <= 0.02:
                continue
            differing += 1
            # Probe a few offsets; if one aligns well, the content moved.
            for dy in (-8, -4, -2, -1, 1, 2, 4, 8):
                if not (0 <= ty + dy and ty + dy + h <= a.height):
                    continue
                alt = 0
                for y in range(h):
                    ra = (ty + y) * a.width
                    rb = (ty + dy + y) * b.width
                    for x in range(tx, tx + w):
                        alt += abs(a.gray[ra + x] - b.gray[rb + x])
                if alt / (w * h) / 255.0 < mean * 0.4:
                    shifted += 1
                    break
    return worst, differing, shifted


def compare(actual: Image, expected: Image) -> dict:
    if actual.width != expected.width or actual.height != expected.height:
        return {
            "dimensions_match": False,
            "actual_size": f"{actual.width}x{actual.height}",
            "expected_size": f"{expected.width}x{expected.height}",
        }
    n = len(actual.gray)
    differing = total = 0
    for i in range(n):
        d = abs(actual.gray[i] - expected.gray[i])
        total += d
        if d > DIFF_TOLERANCE:
            differing += 1
    worst_tile, diff_tiles, shifted_tiles = tile_metrics(actual, expected)
    shift, _ = best_vertical_shift(actual, expected)
    ink_actual = ink_fraction(actual)
    ink_expected = ink_fraction(expected)
    # Relative ink matters more than absolute. A page of text that is 99% whitespace
    # can lose most of its content while moving absolute ink by a fraction of a
    # percent, so an absolute threshold would call that a match.
    ink_ratio = (ink_actual / ink_expected) if ink_expected > 1e-9 else (
        1.0 if ink_actual <= 1e-9 else float("inf"))
    return {
        "dimensions_match": True,
        "size": f"{actual.width}x{actual.height}",
        "differing_fraction": differing / n,
        "mean_abs_error": total / n / 255.0,
        "max_tile_error": worst_tile,
        "differing_tiles": diff_tiles,
        "shifted_tiles": shifted_tiles,
        "ink_delta": ink_actual - ink_expected,
        "ink_ratio": ink_ratio,
        "row_profile_shift": shift,
    }


def diagnose(m: dict) -> str:
    """Turn the numbers into the likely cause. See the table in SKILL.md."""
    if not m["dimensions_match"]:
        return ("DIMENSION MISMATCH - page geometry was read wrongly. Fix this before "
                "looking at anything else; all other metrics are meaningless.")
    # MATCH must also require that no single tile is badly wrong: a whole-page average
    # happily hides one ruined region on an otherwise-blank page.
    if (m["differing_fraction"] < 0.005 and m["mean_abs_error"] < 0.004
            and m["max_tile_error"] < 0.05 and 0.98 <= m["ink_ratio"] <= 1.02):
        return "MATCH - differences are at antialiasing level."
    # Checked before reflow: when content is simply absent, the shift heuristic often
    # also fires (blank rows align with other blank rows), which would misdiagnose it.
    if m["ink_ratio"] < 0.92:
        return (f"CONTENT MISSING - Paperless drew {(1 - m['ink_ratio']) * 100:.0f}% "
                f"less ink than the reference. Look for an unimplemented feature "
                f"(shape type, vector image, fill).")
    if m["ink_ratio"] > 1.08:
        return (f"EXTRA CONTENT - Paperless drew {(m['ink_ratio'] - 1) * 100:.0f}% more "
                f"ink than the reference. Something is visible that should not be, or a "
                f"fill should have been transparent.")
    if m["shifted_tiles"] >= 3 or abs(m["row_profile_shift"]) > 2:
        return (f"REFLOW CASCADE - content is present but moved "
                f"(vertical shift ~{m['row_profile_shift']}px). Suspect layout: font "
                f"metrics, margins or line breaking. Find the FIRST divergence; the "
                f"rest is downstream of it.")
    if m["max_tile_error"] > 0.15 and m["mean_abs_error"] < 0.02:
        return ("LOCALISED DIFFERENCE - one small region is badly wrong. Usually a "
                "colour, a border, or a single shape.")
    return "DIFFERS - no single signature dominates; inspect the diff image."


# ------------------------------------------------------------------------ diff output

def write_png_gray(path: pathlib.Path, width: int, height: int, gray: bytearray) -> None:
    raw = bytearray()
    for y in range(height):
        raw.append(0)                                  # filter type 0 (None)
        raw += gray[y * width:(y + 1) * width]

    def chunk(tag: bytes, body: bytes) -> bytes:
        return (struct.pack(">I", len(body)) + tag + body
                + struct.pack(">I", zlib.crc32(tag + body) & 0xFFFFFFFF))

    path.write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 0, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(bytes(raw), 6))
        + chunk(b"IEND", b""))


def write_triptych(dest: pathlib.Path, actual: Image, expected: Image) -> None:
    """Reference | Paperless | difference map, side by side with 8px gutters."""
    gap = 8
    h = max(actual.height, expected.height)
    w = expected.width + gap + actual.width + gap + min(actual.width, expected.width)
    out = bytearray([200]) * (w * h)

    def blit(img: Image, x0: int) -> None:
        for y in range(img.height):
            out[y * w + x0:y * w + x0 + img.width] = img.gray[
                y * img.width:(y + 1) * img.width]

    blit(expected, 0)
    blit(actual, expected.width + gap)

    dx = expected.width + gap + actual.width + gap
    dw = min(actual.width, expected.width)
    dh = min(actual.height, expected.height)
    for y in range(dh):
        row = y * w + dx
        for x in range(dw):
            d = abs(actual.at(x, y) - expected.at(x, y))
            # Differences shown dark on white so they read at a glance.
            out[row + x] = 255 - min(255, d * 3) if d > DIFF_TOLERANCE else 255

    write_png_gray(dest, w, h, out)


# ------------------------------------------------------------------------------- main

def page_number(path: pathlib.Path) -> int:
    m = re.search(r"(\d+)(?!.*\d)", path.stem)
    return int(m.group(1)) if m else 0


def collect(directory: pathlib.Path) -> list[pathlib.Path]:
    return sorted(directory.glob("*.png"), key=page_number)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--expected", required=True, type=pathlib.Path,
                    help="Directory of LibreOffice reference PNGs.")
    ap.add_argument("--actual", required=True, type=pathlib.Path,
                    help="Directory of Paperless PNGs.")
    ap.add_argument("--diff-dir", type=pathlib.Path,
                    help="Write side-by-side diff images here.")
    ap.add_argument("--report", type=pathlib.Path, help="Write a markdown report here.")
    ap.add_argument("--fail-over", type=float, default=None, metavar="F",
                    help="Exit non-zero if any page's differing_fraction exceeds F.")
    args = ap.parse_args()

    expected = collect(args.expected)
    actual = collect(args.actual)
    if not expected:
        print(f"no reference PNGs in {args.expected}", file=sys.stderr)
        return 3
    if not actual:
        print(f"no Paperless PNGs in {args.actual}", file=sys.stderr)
        return 3

    if args.diff_dir:
        args.diff_dir.mkdir(parents=True, exist_ok=True)

    lines = ["# Render comparison", "",
             f"- reference: `{args.expected}` ({len(expected)} page(s))",
             f"- actual:    `{args.actual}` ({len(actual)} page(s))", ""]
    if len(expected) != len(actual):
        lines += [f"> **PAGE COUNT MISMATCH**: reference has {len(expected)}, "
                  f"Paperless produced {len(actual)}. Pagination itself is wrong, which "
                  f"is a more fundamental problem than any per-page difference below.",
                  ""]
        print(f"PAGE COUNT MISMATCH: {len(expected)} vs {len(actual)}", file=sys.stderr)

    worst = 0.0
    for i in range(min(len(expected), len(actual))):
        exp_img = read_png(expected[i])
        act_img = read_png(actual[i])
        m = compare(act_img, exp_img)
        verdict = diagnose(m)
        page = i + 1

        lines += [f"## Page {page}", "", f"**{verdict}**", ""]
        if m["dimensions_match"]:
            worst = max(worst, m["differing_fraction"])
            lines += [
                "| metric | value |", "|---|---|",
                f"| size | {m['size']} |",
                f"| differing_fraction | {m['differing_fraction']:.4f} |",
                f"| mean_abs_error | {m['mean_abs_error']:.4f} |",
                f"| max_tile_error | {m['max_tile_error']:.4f} |",
                f"| differing_tiles | {m['differing_tiles']} |",
                f"| shifted_tiles | {m['shifted_tiles']} |",
                f"| ink_delta | {m['ink_delta']:+.4f} |",
                f"| ink_ratio | {m['ink_ratio']:.3f} |",
                f"| row_profile_shift | {m['row_profile_shift']:+d} px |", ""]
        else:
            worst = 1.0
            lines += [f"- reference: {m['expected_size']}",
                      f"- actual:    {m['actual_size']}", ""]

        print(f"page {page}: {verdict}")
        if m["dimensions_match"]:
            print(f"           differing={m['differing_fraction']:.4f} "
                  f"mae={m['mean_abs_error']:.4f} "
                  f"worst_tile={m['max_tile_error']:.4f} "
                  f"shifted={m['shifted_tiles']} "
                  f"ink_ratio={m['ink_ratio']:.3f} "
                  f"shift={m['row_profile_shift']:+d}px")

        if args.diff_dir:
            dest = args.diff_dir / f"page-{page}-diff.png"
            write_triptych(dest, act_img, exp_img)
            lines += [f"Diff image: `{dest}` "
                      f"(left: reference, middle: Paperless, right: difference)", ""]

    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text("\n".join(lines), encoding="utf-8")
        print(f"\nreport: {args.report}")

    if len(expected) != len(actual):
        return 1
    if args.fail_over is not None and worst > args.fail_over:
        print(f"\nworst differing_fraction {worst:.4f} exceeds --fail-over "
              f"{args.fail_over}", file=sys.stderr)
        return 1
    return 0


def checksum(path: pathlib.Path) -> str:
    """Golden-image identity. LibreOffice's rasterised output is byte-deterministic,
    so a changed checksum means the reference genuinely changed."""
    return hashlib.sha256(path.read_bytes()).hexdigest()[:16]


if __name__ == "__main__":
    sys.exit(main())
