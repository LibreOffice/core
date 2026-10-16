#!/usr/bin/env python3
"""How a PDF colour literal rasterises, which is why a shaded document ranks high for nothing.

This tree writes a colour component to four decimals (`PdfSyntax.Component` -> `Number`, which
rounds to 4) and 26.2.4.2 writes ten.  Both name the same byte.  The rasteriser does not agree:
it resolves a literal at or below `v/255` to `v - 1`, so every shaded area the two render comes
out one grey level apart although neither renderer is wrong about the colour.

Each arm is a four-object PDF holding one filled square, so nothing but the literal varies.
"""
import pathlib, sys

LITERALS = [
    "0.502",                 # ours for 128: 128.01/255, above the value
    "0.5019607843",          # 26.2.4.2's for 128: a hair below
    "0.50196078431372549",   # 128/255 to the last place a double holds
    "0.851",                 # ours for 217
    "0.8509803922",          # 26.2.4.2's for 217
    "0.5",                   # the midpoint, for the rounding rule itself
]


def build(path, literal):
    content = f"{literal} {literal} {literal} rg\n0 0 100 100 re f\n".encode()
    objects = [
        b"<< /Type /Catalog /Pages 2 0 R >>",
        b"<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
        b"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 100 100] /Contents 4 0 R >>",
        b"<< /Length " + str(len(content)).encode() + b" >>\nstream\n" + content + b"endstream",
    ]
    out = bytearray(b"%PDF-1.4\n")
    offsets = []
    for number, body in enumerate(objects, 1):
        offsets.append(len(out))
        out += f"{number} 0 obj\n".encode() + body + b"\nendobj\n"
    start = len(out)
    out += f"xref\n0 {len(objects) + 1}\n0000000000 65535 f \n".encode()
    for offset in offsets:
        out += f"{offset:010d} 00000 n \n".encode()
    out += (f"trailer\n<< /Size {len(objects) + 1} /Root 1 0 R >>\n"
            f"startxref\n{start}\n%%EOF\n").encode()
    pathlib.Path(path).write_bytes(bytes(out))


def main():
    import fitz
    scratch = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/quantise.pdf")
    print("literal\ttimes255\trasterises to")
    for literal in LITERALS:
        build(scratch, literal)
        with fitz.open(scratch) as doc:
            pixmap = doc[0].get_pixmap(dpi=72, colorspace=fitz.csGRAY)
            value = pixmap.samples[50 * pixmap.width + 50]
        print(f"{literal}\t{float(literal) * 255:.10f}\t{value}")


if __name__ == "__main__":
    main()
