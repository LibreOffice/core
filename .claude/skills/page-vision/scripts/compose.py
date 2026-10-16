#!/usr/bin/env python3
"""Put two renderings of the same page into one labelled image, sized to be *read*.

    compose.py OURS.png REF.png -o pair.png [--layout auto|side|stack] [--budget 2000]

Why this exists
───────────────
Handing a page to a reviewer — a subagent, or yourself — costs one image, and an image is
downscaled to a fixed pixel budget before anyone sees it. Two pages pasted together share
that one budget, so a careless composite halves the resolution of both halves and the
reviewer reports "the text is too small to tell", which is a fact about the compositor and
not about the document.

So this script does three things a shell one-liner does not:

  * **Picks the arrangement that costs the least resolution.** Two portrait pages side by
    side have a long edge of 2×595 pt; stacked they have 2×842. Side by side is therefore
    the higher-resolution arrangement for portrait pages and stacking is higher for 16:9
    slides. `--layout auto` computes it rather than guessing.
  * **Labels the halves in the image itself.** A reviewer told "left is ours" in a prompt
    will occasionally report the two the wrong way round, and a swapped reading is worse
    than no reading — it inverts the sign of every conclusion. The label travels with the
    pixels.
  * **Reports the effective resolution it achieved**, so a reading taken at 40 dpi is not
    mistaken for one taken at 150.

It is deliberately dependency-free (stdlib `zlib`/`struct` only), like every other script
in these skills, so it runs in a bare container.
"""
from __future__ import annotations

import argparse
import pathlib
import struct
import sys
import zlib

# A 5x7 bitmap font, enough for the labels. Hand-rolled because the alternative is a
# font dependency in a script whose whole job is to avoid depending on anything.
GLYPHS: dict[str, tuple[str, ...]] = {
    "A": ("01110", "10001", "10001", "11111", "10001", "10001", "10001"),
    "B": ("11110", "10001", "10001", "11110", "10001", "10001", "11110"),
    "C": ("01110", "10001", "10000", "10000", "10000", "10001", "01110"),
    "D": ("11110", "10001", "10001", "10001", "10001", "10001", "11110"),
    "E": ("11111", "10000", "10000", "11110", "10000", "10000", "11111"),
    "F": ("11111", "10000", "10000", "11110", "10000", "10000", "10000"),
    "G": ("01110", "10001", "10000", "10111", "10001", "10001", "01111"),
    "H": ("10001", "10001", "10001", "11111", "10001", "10001", "10001"),
    "I": ("11111", "00100", "00100", "00100", "00100", "00100", "11111"),
    "J": ("00111", "00010", "00010", "00010", "00010", "10010", "01100"),
    "K": ("10001", "10010", "10100", "11000", "10100", "10010", "10001"),
    "L": ("10000", "10000", "10000", "10000", "10000", "10000", "11111"),
    "M": ("10001", "11011", "10101", "10101", "10001", "10001", "10001"),
    "N": ("10001", "11001", "10101", "10011", "10001", "10001", "10001"),
    "O": ("01110", "10001", "10001", "10001", "10001", "10001", "01110"),
    "P": ("11110", "10001", "10001", "11110", "10000", "10000", "10000"),
    "Q": ("01110", "10001", "10001", "10001", "10101", "10010", "01101"),
    "R": ("11110", "10001", "10001", "11110", "10100", "10010", "10001"),
    "S": ("01111", "10000", "10000", "01110", "00001", "00001", "11110"),
    "T": ("11111", "00100", "00100", "00100", "00100", "00100", "00100"),
    "U": ("10001", "10001", "10001", "10001", "10001", "10001", "01110"),
    "V": ("10001", "10001", "10001", "10001", "10001", "01010", "00100"),
    "W": ("10001", "10001", "10001", "10101", "10101", "11011", "10001"),
    "X": ("10001", "01010", "00100", "00100", "00100", "01010", "10001"),
    "Y": ("10001", "01010", "00100", "00100", "00100", "00100", "00100"),
    "Z": ("11111", "00001", "00010", "00100", "01000", "10000", "11111"),
    "0": ("01110", "10001", "10011", "10101", "11001", "10001", "01110"),
    "1": ("00100", "01100", "00100", "00100", "00100", "00100", "01110"),
    "2": ("01110", "10001", "00001", "00010", "00100", "01000", "11111"),
    "3": ("11111", "00010", "00100", "00010", "00001", "10001", "01110"),
    "4": ("00010", "00110", "01010", "10010", "11111", "00010", "00010"),
    "5": ("11111", "10000", "11110", "00001", "00001", "10001", "01110"),
    "6": ("00110", "01000", "10000", "11110", "10001", "10001", "01110"),
    "7": ("11111", "00001", "00010", "00100", "01000", "01000", "01000"),
    "8": ("01110", "10001", "10001", "01110", "10001", "10001", "01110"),
    "9": ("01110", "10001", "10001", "01111", "00001", "00010", "01100"),
    " ": ("00000", "00000", "00000", "00000", "00000", "00000", "00000"),
    "-": ("00000", "00000", "00000", "11111", "00000", "00000", "00000"),
    ".": ("00000", "00000", "00000", "00000", "00000", "01100", "01100"),
    ",": ("00000", "00000", "00000", "00000", "01100", "01100", "11000"),
    "(": ("00010", "00100", "01000", "01000", "01000", "00100", "00010"),
    ")": ("01000", "00100", "00010", "00010", "00010", "00100", "01000"),
    "/": ("00001", "00010", "00010", "00100", "01000", "01000", "10000"),
    ":": ("00000", "01100", "01100", "00000", "01100", "01100", "00000"),
    "#": ("01010", "11111", "01010", "01010", "01010", "11111", "01010"),
    "%": ("11001", "11010", "00010", "00100", "01000", "01011", "10011"),
    "+": ("00000", "00100", "00100", "11111", "00100", "00100", "00000"),
    "?": ("01110", "10001", "00001", "00010", "00100", "00000", "00100"),
    "!": ("00100", "00100", "00100", "00100", "00100", "00000", "00100"),
}

BAND = 34          # height of the label band, in pixels
DIVIDER = 6        # width of the rule between the two halves


class Image:
    """An 8-bit RGB raster, kept as a flat bytearray."""

    __slots__ = ("width", "height", "rgb")

    def __init__(self, width: int, height: int, rgb: bytearray) -> None:
        self.width, self.height, self.rgb = width, height, rgb

    @classmethod
    def blank(cls, width: int, height: int, shade: int = 255) -> "Image":
        return cls(width, height, bytearray([shade]) * (width * height * 3))

    def paste(self, other: "Image", x: int, y: int) -> None:
        for row in range(other.height):
            src = row * other.width * 3
            dst = ((y + row) * self.width + x) * 3
            self.rgb[dst:dst + other.width * 3] = other.rgb[src:src + other.width * 3]

    def fill(self, x: int, y: int, w: int, h: int, colour: tuple[int, int, int]) -> None:
        run = bytes(colour) * w
        for row in range(y, y + h):
            dst = (row * self.width + x) * 3
            self.rgb[dst:dst + w * 3] = run


def read_png(path: pathlib.Path) -> Image:
    """Decode the 8-bit non-interlaced PNGs that `pdftoppm` emits, and nothing else.

    Anything outside that subset raises rather than being silently mis-decoded — a
    plausible-looking wrong image is the worst thing a diagnostic can hand a reviewer.
    """
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path}: not a PNG")

    pos, idat, palette = 8, bytearray(), b""
    width = height = depth = colour = interlace = -1
    while pos < len(data):
        (length,) = struct.unpack(">I", data[pos:pos + 4])
        kind = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b"IHDR":
            width, height, depth, colour, _, _, interlace = struct.unpack(">IIBBBBB", body)
        elif kind == b"PLTE":
            palette = body
        elif kind == b"IDAT":
            idat += body
        elif kind == b"IEND":
            break

    if depth != 8 or interlace != 0:
        raise ValueError(f"{path}: only 8-bit non-interlaced PNG is supported "
                         f"(got depth {depth}, interlace {interlace})")
    channels = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}.get(colour)
    if channels is None:
        raise ValueError(f"{path}: unsupported colour type {colour}")

    raw = zlib.decompress(bytes(idat))
    stride = width * channels
    out = bytearray(height * stride)
    prev = bytearray(stride)
    at = 0
    for row in range(height):
        filt = raw[at]
        line = bytearray(raw[at + 1:at + 1 + stride])
        at += 1 + stride
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
                a = line[i - channels] if i >= channels else 0
                b = prev[i]
                c = prev[i - channels] if i >= channels else 0
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pred) & 0xFF
        elif filt != 0:
            raise ValueError(f"{path}: bad row filter {filt}")
        out[row * stride:(row + 1) * stride] = line
        prev = line

    rgb = bytearray(width * height * 3)
    for i in range(width * height):
        if colour == 2:
            rgb[i * 3:i * 3 + 3] = out[i * 3:i * 3 + 3]
        elif colour == 6:
            rgb[i * 3:i * 3 + 3] = out[i * 4:i * 4 + 3]
        elif colour == 0:
            rgb[i * 3:i * 3 + 3] = bytes((out[i],)) * 3
        elif colour == 4:
            rgb[i * 3:i * 3 + 3] = bytes((out[i * 2],)) * 3
        else:
            j = out[i] * 3
            rgb[i * 3:i * 3 + 3] = palette[j:j + 3]
    return Image(width, height, rgb)


def write_png(path: pathlib.Path, img: Image) -> None:
    raw = bytearray()
    for row in range(img.height):
        raw.append(0)
        raw += img.rgb[row * img.width * 3:(row + 1) * img.width * 3]

    def chunk(kind: bytes, body: bytes) -> bytes:
        return (struct.pack(">I", len(body)) + kind + body
                + struct.pack(">I", zlib.crc32(kind + body) & 0xFFFFFFFF))

    path.write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", img.width, img.height, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(bytes(raw), 6))
        + chunk(b"IEND", b""))


def scale(img: Image, factor: float) -> Image:
    """Darkest-pixel block reduction: the ink in each source block wins.

    **This was nearest-neighbour and that was wrong, measurably.** The comment beside it
    claimed nearest preserved a one-pixel hairline. It does not — nearest *samples* one
    source pixel per destination pixel, so a rule thinner than the sampling step falls
    between samples and vanishes outright.

    That is not hypothetical. Composing at 80% dropped a real underline — a 245 px solid
    black run at y=802 in the 150 dpi render, confirmed as a fill in the PDF itself — and
    two independent reviewers then reported the underline as absent from the composed
    image. The defect was in this function, and it was reported upward as a defect in the
    renderer. An instrument that loses ink while hunting for missing ink is worse than no
    instrument.

    Averaging is not the answer either: it turns a doubled hairline — a defect this project
    *is* hunting — into one grey smudge indistinguishable from a single line. Taking the
    darkest pixel of each block keeps any hairline that exists at full strength, keeps two
    adjacent ones distinguishable as long as a pale row separates them, and never invents
    ink that was not there. It biases towards showing marks, which is the right bias for a
    reviewer being asked "is this drawn or not".
    """
    if factor >= 1.0:
        return img

    w, h = max(1, int(img.width * factor)), max(1, int(img.height * factor))
    out = bytearray(w * h * 3)
    step = 1.0 / factor

    for y in range(h):
        y0 = int(y * step)
        y1 = max(y0 + 1, min(img.height, int((y + 1) * step)))
        for x in range(w):
            x0 = int(x * step)
            x1 = max(x0 + 1, min(img.width, int((x + 1) * step)))

            darkest, at = 1 << 30, y0 * img.width * 3 + x0 * 3
            for sy in range(y0, y1):
                row = sy * img.width * 3
                for sx in range(x0, x1):
                    i = row + sx * 3
                    # Rec. 601 luma, in integers. Averaging the channels instead would call
                    # saturated yellow "ink" — the error look.py's ink mask made on its first
                    # run, where a periwinkle-to-yellow page scored 0.03% different when it
                    # was 33%.
                    luma = 299 * img.rgb[i] + 587 * img.rgb[i + 1] + 114 * img.rgb[i + 2]
                    if luma < darkest:
                        darkest, at = luma, i

            out[(y * w + x) * 3:(y * w + x) * 3 + 3] = img.rgb[at:at + 3]

    return Image(w, h, out)


def draw_text(img: Image, text: str, x: int, y: int, size: int,
              colour: tuple[int, int, int]) -> None:
    for char in text.upper():
        rows = GLYPHS.get(char, GLYPHS["?"])
        for ry, bits in enumerate(rows):
            for rx, bit in enumerate(bits):
                if bit == "1":
                    img.fill(x + rx * size, y + ry * size, size, size, colour)
        x += 6 * size


def label_band(width: int, text: str, colour: tuple[int, int, int]) -> Image:
    band = Image.blank(width, BAND, 255)
    band.fill(0, 0, width, BAND, colour)
    draw_text(band, text, 8, (BAND - 7 * 3) // 2, 3, (255, 255, 255))
    return band


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("ours")
    ap.add_argument("reference")
    ap.add_argument("-o", "--out", required=True, type=pathlib.Path)
    ap.add_argument("--layout", choices=("auto", "side", "stack"), default="auto")
    ap.add_argument("--budget", type=int, default=2000,
                    help="longest edge the viewer will show without downscaling")
    ap.add_argument("--left-label", default="OURS (X-RAY.OFFICE)")
    ap.add_argument("--right-label", default="REFERENCE (LIBREOFFICE)")
    a = ap.parse_args()

    ours, ref = read_png(pathlib.Path(a.ours)), read_png(pathlib.Path(a.reference))

    # Two halves rendered at different dpi compose into an image whose most striking
    # feature is the scale difference, and a reviewer reports that as "our text is much
    # smaller" — a defect that exists only in the compositor. `compare-images.py` refuses
    # outright on this; here it is a loud warning rather than an error, because a genuine
    # page-size difference is itself worth looking at.
    if abs(ours.width - ref.width) > 2 or abs(ours.height - ref.height) > 2:
        print(f"WARNING: halves differ in size — ours {ours.width}x{ours.height}, "
              f"reference {ref.width}x{ref.height}.", file=sys.stderr)
        print("  If you rendered them at different dpi, fix that and re-run: the reviewer "
              "will otherwise report the scale difference as the finding.", file=sys.stderr)
        print("  If both were rendered at the same dpi, the page geometry genuinely "
              "differs and THAT is the finding.", file=sys.stderr)

    layout = a.layout
    if layout == "auto":
        # Whichever arrangement has the shorter long edge keeps more resolution after
        # the viewer's downscale. For portrait pages that is side by side; for 16:9
        # slides it is stacking.
        side_edge = max(ours.width + ref.width + DIVIDER, max(ours.height, ref.height) + BAND)
        stack_edge = max(max(ours.width, ref.width), ours.height + ref.height + DIVIDER + 2 * BAND)
        layout = "side" if side_edge <= stack_edge else "stack"

    if layout == "side":
        height = max(ours.height, ref.height) + BAND
        width = ours.width + DIVIDER + ref.width
        out = Image.blank(width, height, 255)
        out.paste(label_band(ours.width, a.left_label, (30, 80, 170)), 0, 0)
        out.paste(label_band(ref.width, a.right_label, (190, 90, 20)), ours.width + DIVIDER, 0)
        out.fill(ours.width, 0, DIVIDER, height, (0, 0, 0))
        out.paste(ours, 0, BAND)
        out.paste(ref, ours.width + DIVIDER, BAND)
    else:
        width = max(ours.width, ref.width)
        height = ours.height + ref.height + DIVIDER + 2 * BAND
        out = Image.blank(width, height, 255)
        out.paste(label_band(width, a.left_label, (30, 80, 170)), 0, 0)
        out.paste(ours, 0, BAND)
        out.fill(0, BAND + ours.height, width, DIVIDER, (0, 0, 0))
        out.paste(label_band(width, a.right_label, (190, 90, 20)),
                  0, BAND + ours.height + DIVIDER)
        out.paste(ref, 0, 2 * BAND + ours.height + DIVIDER)

    long_edge = max(out.width, out.height)
    factor = min(1.0, a.budget / long_edge)
    final = scale(out, factor)
    write_png(a.out, final)

    print(f"{a.out}  {final.width}x{final.height}  layout={layout}  "
          f"composed={out.width}x{out.height}  shown at {factor * 100:.0f}% of composed")
    if factor < 1.0:
        print(f"  each half is {factor * 100:.0f}% of the size it was rendered at — "
              f"render at ~{factor * 100:.0f}% of the dpi you would have used, or crop")
    return 0


if __name__ == "__main__":
    sys.exit(main())
