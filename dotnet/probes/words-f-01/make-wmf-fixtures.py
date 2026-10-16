#!/usr/bin/env python3
"""Build this round's fixtures: an inline picture whose `pib` collides, and a cropped WMF.

    make-wmf-fixtures.py <outdir>
    soffice --headless --convert-to doc --outdir <fresh> <outdir>/picture-blip-collision.docx
    soffice --headless --convert-to doc --outdir <fresh> <outdir>/picture-crop-wmf.docx

Both are newly authored, element by element - no corpus document is copied or excerpted.
The `.docx` is the input and the `.doc` is what the tests read, because LibreOffice's own
export is what turns an `a:srcRect` into Escher's cropFromTop/Bottom/Left/Right (256-259)
and puts an inline shape in the `Data` stream where Word puts one. Both are then patched
into the shape Word writes - the PICF crop zeroed and the goal shrunk to the visible extent
- by the same edit and for the same reason as `picture-crop-goal.doc`.

`picture-blip-collision.docx` is the one the tests use. It holds an **anchored** PNG, which
is what puts an entry in the document's shared blip store, and an **inline** PNG whose `pib`
is numbered from one inside its own container and therefore names that entry too. The two
are 100 x 100 and 64 x 64 pixels, and the difference in size is the whole instrument: both
are drawn at the same place whichever way the `pib` is read, so the pixel count is the only
thing that says which one arrived.

`picture-crop-wmf.docx` is the same shape with a hand-assembled WMF inline. **It is not used
by any test**, because it refutes rather than supports the rule it was built for: 26.2.4.2
applies its Escher crop, while it ignores the crop on every WMF in `150_5300_13_chg10.doc`,
`150_5300_13_chg12.doc` and `150_5300_13_chg8.doc`. See `results.md` section 4. It is kept
because reproducing that contradiction is the next round's starting point.
"""
import os
import struct
import sys
import zipfile
import zlib

A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PIC = 'http://schemas.openxmlformats.org/drawingml/2006/picture'
WP = 'http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing'

CX, CY = 3657600, 2743200          # 288 x 216 pt
SRCRECT = '<a:srcRect l="10000" t="20000" r="30000" b="40000"/>'


def png(w, h, rows):
    def chunk(t, d):
        c = t + d
        return struct.pack('>I', len(d)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)

    raw = b''.join(b'\x00' + bytes(r) for r in rows)
    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(raw))
            + chunk(b'IEND', b''))


def quadrants(side):
    rows = []
    for y in range(side):
        row = []
        for x in range(side):
            row += [255 if x < side // 2 else 0, 255 if y < side // 2 else 0, 128]
        rows.append(row)
    return png(side, side, rows)


# Two sizes, and the difference is the whole instrument: the two pictures are drawn at the
# same place whichever way the `pib` is read, so the *pixel* size is the only thing that says
# which one arrived.
FLOATING_SIDE = 100
INLINE_SIDE = 64
IMAGE = quadrants(FLOATING_SIDE)
INLINE_IMAGE = quadrants(INLINE_SIDE)


def wmf():
    """A placeable WMF: four filled quadrants in a 100 x 100 window.

    Hand-assembled rather than converted from anything, so that the bytes are known and the
    fixture carries no third-party content. Four quadrants for the same reason the PNG has
    them — a crop that takes 10% off one side and 30% off the other is visible in the result
    rather than merely arithmetically present.
    """
    records = []

    def record(func, params):
        body = b''.join(struct.pack('<h', p) for p in params)
        records.append(struct.pack('<IH', 3 + len(body) // 2, func) + body)

    record(0x020B, [0, 0])              # SetWindowOrg  y, x
    record(0x020C, [100, 100])          # SetWindowExt  y, x
    for i, (x0, y0, colour) in enumerate((
            (0, 0, 0x0000FF), (50, 0, 0x00FF00), (0, 50, 0xFF0000), (50, 50, 0x808080))):
        # CreateBrushIndirect: style 0 (solid), colour, hatch
        records.append(struct.pack('<IHHIH', 7, 0x02FC, 0, colour, 0))
        record(0x012D, [i])             # SelectObject
        record(0x041B, [y0 + 50, x0 + 50, y0, x0])   # Rectangle  bottom, right, top, left
    records.append(struct.pack('<IH', 3, 0x0000))    # EOF

    body = b''.join(records)
    largest = max(len(r) for r in records) // 2
    header = struct.pack('<HHHIHIH', 1, 9, 0x0300,
                         (18 + len(body)) // 2, 4, largest, 0)

    placeable = struct.pack('<IHhhhhHI', 0x9AC6CDD7, 0, 0, 0, 100, 100, 100, 0)
    checksum = 0
    for i in range(0, 20, 2):
        checksum ^= struct.unpack_from('<H', placeable, i)[0]
    return placeable + struct.pack('<H', checksum) + header + body


WMF = wmf()

CONTENT_TYPES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Default Extension="wmf" ContentType="image/x-wmf"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>'''

ROOT_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''


def doc_rels(with_png):
    image = ('<Relationship Id="rId8" Type="http://schemas.openxmlformats.org/officeDocument/'
             '2006/relationships/image" Target="media/image1.png"/>') if with_png else ''
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            + image
            + '<Relationship Id="rId7" Type="http://schemas.openxmlformats.org/officeDocument/'
              '2006/relationships/image" Target="media/image3.png"/>'
            + '<Relationship Id="rId9" Type="http://schemas.openxmlformats.org/officeDocument/'
              '2006/relationships/image" Target="media/image2.wmf"/></Relationships>')


INLINE_WMF = f'''<w:p><w:r><w:drawing>
<wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="{CX}" cy="{CY}"/>
<wp:docPr id="1" name="CroppedMetafile"/>
<a:graphic><a:graphicData uri="{PIC}">
<pic:pic><pic:nvPicPr><pic:cNvPr id="1" name="CroppedMetafile"/><pic:cNvPicPr/></pic:nvPicPr>
<pic:blipFill><a:blip r:embed="rId7"/>{SRCRECT}<a:stretch><a:fillRect/></a:stretch></pic:blipFill>
<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{CX}" cy="{CY}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr>
</pic:pic></a:graphicData></a:graphic>
</wp:inline>
</w:drawing></w:r></w:p>'''

# Anchored, so its blip lands in the document's shared store rather than beside the shape.
ANCHORED_PNG = f'''<w:p><w:r><w:drawing>
<wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="1"
 behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
<wp:simplePos x="0" y="0"/>
<wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>
<wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>
<wp:extent cx="914400" cy="914400"/>
<wp:wrapNone/><wp:docPr id="2" name="Floating"/>
<a:graphic><a:graphicData uri="{PIC}">
<pic:pic><pic:nvPicPr><pic:cNvPr id="2" name="Floating"/><pic:cNvPicPr/></pic:nvPicPr>
<pic:blipFill><a:blip r:embed="rId8"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>
<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="914400" cy="914400"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr>
</pic:pic></a:graphicData></a:graphic>
</wp:anchor>
</w:drawing></w:r></w:p>'''


def document(body):
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
            f' xmlns:r="{R}" xmlns:wp="{WP}" xmlns:a="{A}" xmlns:pic="{PIC}"><w:body>{body}'
            f'<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
            f'<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
            f' w:header="720" w:footer="720" w:gutter="0"/></w:sectPr></w:body></w:document>')


def write(path, members):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, body in members:
            z.writestr(name, body)
    print('wrote', path, os.path.getsize(path), 'bytes')


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '.'

    write(os.path.join(out, 'picture-crop-wmf.docx'), [
        ('[Content_Types].xml', CONTENT_TYPES),
        ('_rels/.rels', ROOT_RELS),
        ('word/document.xml', document(INLINE_WMF)),
        ('word/_rels/document.xml.rels', doc_rels(with_png=False)),
        ('word/media/image2.wmf', WMF),
        ('word/media/image3.png', INLINE_IMAGE),
    ])

    write(os.path.join(out, 'picture-blip-collision.docx'), [
        ('[Content_Types].xml', CONTENT_TYPES),
        ('_rels/.rels', ROOT_RELS),
        ('word/document.xml', document(ANCHORED_PNG + INLINE_WMF)),
        ('word/_rels/document.xml.rels', doc_rels(with_png=True)),
        ('word/media/image1.png', IMAGE),
        ('word/media/image2.wmf', WMF),
        ('word/media/image3.png', INLINE_IMAGE),
    ])


if __name__ == '__main__':
    main()
