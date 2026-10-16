#!/usr/bin/env python3
"""24.2.7.2 audit: is `a:lum`'s brightness read as a WHOLE per cent, truncated?

`XRay.Office.Ooxml/DrawingML/DrawingFill.cs:115` states, of 24.2.7.2:

    getLimitedValue<sal_Int16>(value / PER_PERCENT, -100, 100)
    (oox/source/drawingml/fillproperties.cxx:799-800)

and that "the branch that recognises PowerPoint's washout tests that integer for exactly 70
and -70".  `PER_PERCENT` is 1000 and the division is integer, so the claim is that the
attribute is **truncated** to whole per cent and not rounded.

The discriminator does not need our renderer at all — it is a property of the reference:

    bright= 70000 contrast= -70000   ->  70 / -70   -> WATERMARK on either reading
    bright= 70999 contrast= -70999   ->  truncation gives 70 / -70  -> WATERMARK
                                         rounding   gives 71 / -71  -> applyBrightnessContrast
    bright= 69500 contrast= -69500   ->  truncation gives 69 / -69  -> applyBrightnessContrast
                                         rounding   gives 70 / -70  -> WATERMARK

So `70999` renders identically to `70000` under truncation and differently under rounding,
and `69500` does the opposite.  Two cases that disagree under the two readings in *opposite*
directions, plus two controls whose answer is known before the probe runs: `0/0` (no
adjustment at all) and `70000/-70000` itself.

The image is a saturated red/blue checkerboard authored here, so the washout — which maps to
`ColorMode_WATERMARK`, a fixed pale wash — is separable from `applyBrightnessContrast` by mean
luminance alone; no pixel-perfect comparison is needed.

Refuses to summarise unless all five cases rendered.
"""
import os
import shutil
import struct
import subprocess
import sys
import zipfile
import zlib

OUT = "/c/sandbox/workdir/scratch-r61-sheets/lum"

CASES = [("zero", 0, 0), ("w70000", 70000, -70000), ("w70999", 70999, -70999),
         ("w69500", 69500, -69500), ("w71000", 71000, -71000)]


def png(width, height):
    """A saturated red/blue checkerboard, 8-bit RGB, no filtering."""
    rows = b""
    for y in range(height):
        row = b"\x00"
        for x in range(width):
            row += b"\xff\x00\x00" if (x // 8 + y // 8) % 2 == 0 else b"\x00\x00\xff"
        rows += row

    def chunk(kind, data):
        return (struct.pack(">I", len(data)) + kind + data
                + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF))

    return (b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(rows, 9))
            + chunk(b"IEND", b""))


DOC = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture">
<w:body><w:p><w:r><w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="3600000" cy="3600000"/><wp:docPr id="1" name="p"/>
<a:graphic><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">
<pic:pic><pic:nvPicPr><pic:cNvPr id="1" name="p"/><pic:cNvPicPr/></pic:nvPicPr>
<pic:blipFill><a:blip r:embed="rId9">%s</a:blip><a:stretch><a:fillRect/></a:stretch></pic:blipFill>
<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="3600000" cy="3600000"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>
</a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""

TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId9" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/image1.png"/>
</Relationships>"""


def build(name, bright, contrast):
    lum = "" if bright == 0 else '<a:lum bright="%d" contrast="%d"/>' % (bright, contrast)
    path = os.path.join(OUT, name + ".docx")
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/document.xml", DOC % lum)
        z.writestr("word/media/image1.png", png(64, 64))
    return path


def render(name, doc):
    d = os.path.join(OUT, "r-" + name)
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    subprocess.run(["soffice", "-env:UserInstallation=file://" + os.path.join(OUT, "prof"),
                    "--headless", "--convert-to", "pdf", "--outdir", d, doc],
                   capture_output=True, timeout=600)
    return os.path.join(d, name + ".pdf")


def ink(pdf):
    """Mean red and mean blue over the page, at 60 dpi — enough to separate a wash."""
    d = os.path.dirname(pdf)
    subprocess.run(["pdftoppm", "-r", "60", "-png", "-f", "1", "-l", "1", pdf,
                    os.path.join(d, "page")], capture_output=True, timeout=300)
    pngs = [f for f in os.listdir(d) if f.startswith("page") and f.endswith(".png")]
    if not pngs:
        return None
    raw = subprocess.run(["python3", "-c", """
import sys, zlib, struct
data = open(sys.argv[1], 'rb').read()
at, w, h, idat = 8, 0, 0, b''
while at < len(data):
    ln = struct.unpack('>I', data[at:at+4])[0]; kind = data[at+4:at+8]
    body = data[at+8:at+8+ln]
    if kind == b'IHDR': w, h, depth, ctype = struct.unpack('>IIBB', body[:10])
    if kind == b'IDAT': idat += body
    at += 12 + ln
px = zlib.decompress(idat)
stride = w*3 if ctype == 2 else w
prev = bytearray(stride); out = []
i = 0
for y in range(h):
    f = px[i]; i += 1
    line = bytearray(px[i:i+stride]); i += stride
    bpp = 3 if ctype == 2 else 1
    for x in range(stride):
        a = line[x-bpp] if x >= bpp else 0
        b = prev[x]
        c = prev[x-bpp] if x >= bpp else 0
        if f == 1: line[x] = (line[x] + a) & 255
        elif f == 2: line[x] = (line[x] + b) & 255
        elif f == 3: line[x] = (line[x] + (a+b)//2) & 255
        elif f == 4:
            p = a + b - c; pa, pb, pc = abs(p-a), abs(p-b), abs(p-c)
            pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
            line[x] = (line[x] + pr) & 255
    out.append(bytes(line)); prev = line
if ctype == 2:
    r = sum(l[0::3][j] for l in out for j in range(w))
    g = sum(l[1::3][j] for l in out for j in range(w))
    b = sum(l[2::3][j] for l in out for j in range(w))
    n = w*h
    print('%.3f %.3f %.3f' % (r/n, g/n, b/n))
else:
    n = w*h
    print('%.3f %.3f %.3f' % tuple([sum(sum(l) for l in out)/n]*3))
""", os.path.join(d, pngs[0])], capture_output=True, text=True)
    try:
        return tuple(float(v) for v in raw.stdout.split())
    except ValueError:
        return None


def main():
    os.makedirs(OUT, exist_ok=True)
    rows, bad = [], []
    for name, bright, contrast in CASES:
        got = ink(render(name, build(name, bright, contrast)))
        if got is None:
            bad.append(name)
            continue
        rows.append((name, bright, got))
    if bad:
        print("REFUSING TO SUMMARISE — no rendering for:", bad)
        return 2
    print("%-8s %8s   %8s %8s %8s" % ("case", "bright", "meanR", "meanG", "meanB"))
    for name, bright, (r, g, b) in rows:
        print("%-8s %8d   %8.3f %8.3f %8.3f" % (name, bright, r, g, b))
    by = {name: got for name, _, got in rows}

    def same(a, b, tol=0.5):
        return all(abs(x - y) < tol for x, y in zip(by[a], by[b]))

    print()
    print("70999 renders as 70000 (truncation) :", same("w70999", "w70000"))
    print("69500 renders as 70000 (rounding)   :", same("w69500", "w70000"))
    print("71000 renders as 70000 (control, must be False):", same("w71000", "w70000"))
    print("0     renders as 70000 (control, must be False):", same("zero", "w70000"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
