#!/usr/bin/env python3
"""Our pattern tiles beside the ones 26.2.4.2 resolves for the same workbook.

    tile-check.py <ours.pdf> <reference.fods>

O40's claim is that an Escher `mso_fillPattern` is a two-colour stencil and that recolouring it
the way `DffPropertyReader::ApplyFillAttributes` does gives the tile the reference draws. This
checks that against the reference's *own answer* rather than against a rendering: converting the
workbook with `soffice --convert-to fods` writes each resolved fill out as a `draw:fill-image`
PNG, and our renderer writes each as an 8x8 `DeviceRGB` image XObject in its PDF. Both are the
recoloured tile, so they can be compared pixel for pixel with no rasteriser in between.

It also prints the alpha each side states — `/ca` in an `ExtGState` for us, `draw:opacity` on the
graphic style for the reference — because `fillOpacity` is the other half of the fill.

Neither PDF nor PNG decoding here needs a codec: both stores are raw Flate over 8-bit RGB.
"""
import base64
import re
import struct
import sys
import zlib

SIDE = 8


def ours(pdf):
    """Every 8x8 DeviceRGB image XObject in the PDF, as rows of `#rrggbb`, in object order."""
    data = open(pdf, "rb").read()
    tiles = []
    for found in re.finditer(
            rb"(\d+) 0 obj\s*<<([^>]*?/Subtype\s*/Image.*?)>>\s*stream\r?\n", data, re.S):
        header = found.group(2)
        if b"/Width 8" not in header or b"/Height 8" not in header:
            continue
        start = found.end()
        pixels = zlib.decompress(data[start:data.index(b"endstream", start)])
        tiles.append((int(found.group(1)), grid(pixels)))
    return tiles


def reference(fods):
    """Every `draw:fill-image` in the flat ODF, decoded the same way."""
    text = open(fods, encoding="utf-8").read()
    out = {}
    for found in re.finditer(
            r'<draw:fill-image draw:name="([^"]*)".*?'
            r"<office:binary-data>(.*?)</office:binary-data>", text, re.S):
        png = base64.b64decode(re.sub(r"\s", "", found.group(2)))
        out[found.group(1)] = grid(unfilter(idat(png)))
    return out


def idat(png):
    """The concatenated IDAT payload of a PNG, inflated."""
    at, parts = 8, []
    while at < len(png):
        length = struct.unpack(">I", png[at:at + 4])[0]
        kind, body = png[at + 4:at + 8], png[at + 8:at + 8 + length]
        at += 12 + length
        if kind == b"IDAT":
            parts.append(body)
    return zlib.decompress(b"".join(parts))


def unfilter(raw):
    """An 8x8 truecolour PNG's scanlines, with the per-row filters undone."""
    stride, out, previous, at = SIDE * 3, bytearray(), bytearray(SIDE * 3), 0
    for _ in range(SIDE):
        kind = raw[at]
        at += 1
        line = bytearray(raw[at:at + stride])
        at += stride
        for i in range(stride):
            a = line[i - 3] if i >= 3 else 0
            b = previous[i]
            c = previous[i - 3] if i >= 3 else 0
            if kind == 1:
                line[i] = (line[i] + a) & 255
            elif kind == 2:
                line[i] = (line[i] + b) & 255
            elif kind == 3:
                line[i] = (line[i] + ((a + b) // 2)) & 255
            elif kind == 4:
                pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - (2 * c))
                near = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + near) & 255
        out += line
        previous = line
    return bytes(out)


def grid(pixels):
    return [["#%02x%02x%02x" % tuple(pixels[((y * SIDE) + x) * 3:(((y * SIDE) + x) * 3) + 3])
             for x in range(SIDE)] for y in range(SIDE)]


def main(pdf, fods):
    theirs = reference(fods)
    mine = ours(pdf)
    print(f"# {len(mine)} tiles in {pdf}, {len(theirs)} in {fods}")
    for number, tile in mine:
        same = [name for name, other in theirs.items() if other == tile]
        print(f"obj {number}\t{'=' if same else 'NO MATCH'}\t{','.join(same) or '-'}"
              f"\t{' '.join(tile[0])}")

    print("# alpha ours   " + " ".join(
        sorted(set(m.group(1) for m in re.finditer(rb"/ca\s+([\d.]+)", open(pdf, "rb").read()))
               and [m.group(1).decode()
                    for m in re.finditer(rb"/ca\s+([\d.]+)", open(pdf, "rb").read())])))
    print("# alpha ref    " + " ".join(sorted(
        m.group(1) for m in re.finditer(r'draw:opacity="([^"]*)"',
                                        open(fods, encoding="utf-8").read()))))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
