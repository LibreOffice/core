"""Build the words and sheets halves of this round's crop fixture pair.

    make-crop-fixtures.py <outdir>
    soffice --headless --convert-to doc --outdir <fresh> <outdir>/picture-crop.docx
    soffice --headless --convert-to xls --outdir <fresh> <outdir>/picture-crop.xlsx

The second step is what produces the `.doc` and the `.xls`: LibreOffice's own export is
what turns an `a:srcRect` into Escher's cropFromTop/Bottom/Left/Right (256-259), and
hand-assembling a WordDocument or a Workbook stream is the only alternative. Use a fresh
--outdir; a convert-to into a directory that already holds output can produce nothing and
still exit 0.

Same picture and same fractions as `slides-c-01`'s `picture-crop.pptx`, deliberately: an
8 x 8 PNG of four quadrants, 288 x 216 pt, 10% off the left, 20% off the top, 30% off the
right and 40% off the bottom. So the whole picture belongs in 288/0.6 = 480 pt by
216/0.4 = 540 pt, offset (-48, -108) from where the frame sits — the same answer the
slide fixture pins, on two more readers.
"""
import os
import struct
import sys
import zipfile
import zlib


def png(w, h, rows):
    def chunk(t, d):
        c = t + d
        return struct.pack('>I', len(d)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)

    raw = b''.join(b'\x00' + bytes(r) for r in rows)
    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(raw))
            + chunk(b'IEND', b''))


# 100 x 100 rather than the slide fixture's 8 x 8, and the size is load-bearing.
# LibreOffice's BIFF export states the crop in the *bitmap's* pixels, so an 8-pixel
# image quantises a 10% crop to 0.8 of a pixel and the fraction comes back as 0.0990
# instead of 0.1000 — measured. At 100 pixels the round trip is exact to four places.
SIDE = 100
rows = []
for y in range(SIDE):
    row = []
    for x in range(SIDE):
        row += [255 if x < SIDE // 2 else 0, 255 if y < SIDE // 2 else 0, 128]
    rows.append(row)
IMAGE = png(SIDE, SIDE, rows)

CX, CY = 3657600, 2743200          # 288 x 216 pt
SRCRECT = '<a:srcRect l="10000" t="20000" r="30000" b="40000"/>'

A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PIC = 'http://schemas.openxmlformats.org/drawingml/2006/picture'

def blipfill(prefix):
    """`a:srcRect` on a blip fill, in whichever namespace the host part uses.

    Spreadsheet drawings put `xdr:pic`'s children in the *xdr* namespace, not the
    picture one: `pic:blipFill` inside an `xdr:pic` is silently ignored, and the
    tell is a converted `.xls` whose picture frame carries no `pib` at all.
    """
    return (f'<{prefix}:blipFill><a:blip r:embed="rId9"/>{SRCRECT}'
            f'<a:stretch><a:fillRect/></a:stretch></{prefix}:blipFill>')

# --------------------------------------------------------------------------- docx

DOCX_CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>'''

ROOT_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="{target}"/>
</Relationships>'''

DOC_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId9" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/image1.png"/>
</Relationships>'''

DOCUMENT = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="{R}" xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="{A}" xmlns:pic="{PIC}">
<w:body>
<w:p><w:r><w:drawing>
<wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="{CX}" cy="{CY}"/>
<wp:docPr id="1" name="Cropped"/>
<a:graphic><a:graphicData uri="{PIC}">
<pic:pic><pic:nvPicPr><pic:cNvPr id="1" name="Cropped"/><pic:cNvPicPr/></pic:nvPicPr>
{blipfill('pic')}
<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{CX}" cy="{CY}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr>
</pic:pic></a:graphicData></a:graphic>
</wp:inline>
</w:drawing></w:r></w:p>
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>
</w:sectPr>
</w:body></w:document>'''

# --------------------------------------------------------------------------- xlsx

XLSX_CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>
</Types>'''

WORKBOOK = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="{R}">
<sheets><sheet name="Sheet1" sheetId="1" r:id="rId1"/></sheets></workbook>'''

WORKBOOK_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
</Relationships>'''

SHEET = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="{R}">
<sheetData><row r="1"><c r="A1" t="inlineStr"><is><t>crop</t></is></c></row></sheetData>
<drawing r:id="rId8"/></worksheet>'''

SHEET_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId8" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing" Target="../drawings/drawing1.xml"/>
</Relationships>'''

DRAWING = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
 xmlns:a="{A}" xmlns:r="{R}" xmlns:pic="{PIC}">
<xdr:oneCellAnchor>
<xdr:from><xdr:col>1</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>3</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>
<xdr:ext cx="{CX}" cy="{CY}"/>
<xdr:pic><xdr:nvPicPr><xdr:cNvPr id="2" name="Cropped"/><xdr:cNvPicPr/></xdr:nvPicPr>
{blipfill('xdr')}
<xdr:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{CX}" cy="{CY}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></xdr:spPr>
</xdr:pic><xdr:clientData/>
</xdr:oneCellAnchor></xdr:wsDr>'''

DRAWING_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId9" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="../media/image1.png"/>
</Relationships>'''


def write(path, members):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, body in members:
            z.writestr(name, body)
    print('wrote', path, os.path.getsize(path), 'bytes,', len(members), 'members')


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '.'

    write(os.path.join(out, 'picture-crop.docx'), [
        ('[Content_Types].xml', DOCX_CT),
        ('_rels/.rels', ROOT_RELS.format(target='word/document.xml')),
        ('word/document.xml', DOCUMENT),
        ('word/_rels/document.xml.rels', DOC_RELS),
        ('word/media/image1.png', IMAGE),
    ])

    write(os.path.join(out, 'picture-crop.xlsx'), [
        ('[Content_Types].xml', XLSX_CT),
        ('_rels/.rels', ROOT_RELS.format(target='xl/workbook.xml')),
        ('xl/workbook.xml', WORKBOOK),
        ('xl/_rels/workbook.xml.rels', WORKBOOK_RELS),
        ('xl/worksheets/sheet1.xml', SHEET),
        ('xl/worksheets/_rels/sheet1.xml.rels', SHEET_RELS),
        ('xl/drawings/drawing1.xml', DRAWING),
        ('xl/drawings/_rels/drawing1.xml.rels', DRAWING_RELS),
        ('xl/media/image1.png', IMAGE),
    ])


if __name__ == '__main__':
    main()


# --------------------------------------------------------------- the Word-shaped variant

def word_shaped(src, dest):
    """`picture-crop-goal.doc`: the same picture with the crop stated only in Escher.

    LibreOffice's DOC export writes the crop TWICE — into the PICF's dxaCropLeft and
    its three siblings, and into Escher properties 256-259 — and sizes dxaGoal to the
    WHOLE picture. Word does not: every one of the 32 cropped inline pictures in the
    corpus has dxaCrop* = 0 and a dxaGoal that is already the *visible* size.

    A round trip through soffice therefore cannot produce the file the corpus is made
    of, and testing only against the round trip is what made this round's first word
    implementation pass its own fixture and be wrong on all seven documents it moved.
    So the exported file is patched into the Word-shaped form: the crop fields go to
    zero and the goal shrinks to the visible extent, which is the same edit in reverse.
    Both fixtures must produce the same 288 x 216 pt frame and the same 480 x 540 pt
    destination, by the two different routes a .doc can state them.

    The patch is byte-for-byte in place — same field widths, no structural change — so
    the OLE2 container is untouched and the file stays exactly as `soffice` wrote it
    everywhere else.
    """
    data = bytearray(open(src, 'rb').read())

    # dxaGoal, dyaGoal, mx, my then the four crops, as the exported file states them.
    old = struct.pack('<hhHHhhhh', 1500, 1500, 6400, 7200, 150, 300, 450, 600)
    # The visible goal, and no PICF crop at all: (1500-150-450) and (1500-300-600).
    new = struct.pack('<hhHHhhhh', 900, 600, 6400, 7200, 0, 0, 0, 0)

    if data.count(old) != 1:
        raise SystemExit(f'expected exactly one PICF to patch, found {data.count(old)}')

    data[data.index(old):data.index(old) + len(old)] = new
    open(dest, 'wb').write(bytes(data))
    print('wrote', dest, len(data), 'bytes')
