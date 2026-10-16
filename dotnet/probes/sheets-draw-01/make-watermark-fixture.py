"""Build `picture-watermark.xlsx`: two sheet pictures, one faded and one not.

    make-watermark-fixture.py <outdir>

The fixture pins `xdr:blipFill/a:blip/a:alphaModFix/@amt` on a spreadsheet drawing, which
is what makes a picture on a sheet a watermark rather than a lid. It is a *pair* on one
sheet on purpose: the second picture states no `alphaModFix` at all, so the test can say
both that the attribute is read and that its absence still means fully opaque. A fixture
with only the faded half would pass equally well against a reader that faded everything.

Both pictures sit over a cell holding text, because the defect this came from is not "the
picture is the wrong colour" — it is that an opaque picture *hides the text under it* while
leaving that text in the PDF's own text layer, so every word-count column reads correct
and the page is unreadable. Measured on `SIL_TDB648.xlsx`, whose General Info sheet states
`amt="20000"` on a photograph spanning eighteen rows.

No round trip through soffice: unlike the crop pair, nothing here needs a binary exporter
to translate the property — `alphaModFix` is stated by the file we write and read by the
SpreadsheetML path directly.
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


# Solid mid-blue, 16 x 16. A flat colour rather than a pattern because what matters is the
# alpha the picture is composited at, and a flat source makes that one subtraction.
SIDE = 16
IMAGE = png(SIDE, SIDE, [[0x1F, 0x4E, 0x9C] * SIDE for _ in range(SIDE)])

CX, CY = 2743200, 914400           # 216 x 72 pt

A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>
</Types>'''

ROOT_RELS = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

WORKBOOK = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="{R}">
<sheets><sheet name="Sheet1" sheetId="1" r:id="rId1"/></sheets></workbook>'''

WORKBOOK_RELS = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/worksheet" Target="worksheets/sheet1.xml"/>
</Relationships>'''

SHEET = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="{R}">
<sheetData>
<row r="1"><c r="A1" t="inlineStr"><is><t>under the watermark</t></is></c></row>
<row r="8"><c r="A8" t="inlineStr"><is><t>under the opaque one</t></is></c></row>
</sheetData>
<drawing r:id="rId8"/></worksheet>'''

SHEET_RELS = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId8" Type="{R}/drawing" Target="../drawings/drawing1.xml"/>
</Relationships>'''


def anchor(row, name, alpha):
    """One `xdr:oneCellAnchor` holding a picture, faded when `alpha` is given."""
    fix = f'<a:alphaModFix amt="{alpha}"/>' if alpha is not None else ''
    return f'''<xdr:oneCellAnchor>
<xdr:from><xdr:col>0</xdr:col><xdr:colOff>0</xdr:colOff>
<xdr:row>{row}</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>
<xdr:ext cx="{CX}" cy="{CY}"/>
<xdr:pic><xdr:nvPicPr><xdr:cNvPr id="{row + 2}" name="{name}"/><xdr:cNvPicPr/></xdr:nvPicPr>
<xdr:blipFill><a:blip r:embed="rId9">{fix}</a:blip>
<a:stretch><a:fillRect/></a:stretch></xdr:blipFill>
<xdr:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{CX}" cy="{CY}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></xdr:spPr>
</xdr:pic><xdr:clientData/>
</xdr:oneCellAnchor>'''


DRAWING = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
 xmlns:a="{A}" xmlns:r="{R}">
{anchor(0, 'Watermark', 20000)}
{anchor(7, 'Opaque', None)}
</xdr:wsDr>'''

DRAWING_RELS = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId9" Type="{R}/image" Target="../media/image1.png"/>
</Relationships>'''


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    path = os.path.join(out, 'picture-watermark.xlsx')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, body in [
            ('[Content_Types].xml', CT),
            ('_rels/.rels', ROOT_RELS),
            ('xl/workbook.xml', WORKBOOK),
            ('xl/_rels/workbook.xml.rels', WORKBOOK_RELS),
            ('xl/worksheets/sheet1.xml', SHEET),
            ('xl/worksheets/_rels/sheet1.xml.rels', SHEET_RELS),
            ('xl/drawings/drawing1.xml', DRAWING),
            ('xl/drawings/_rels/drawing1.xml.rels', DRAWING_RELS),
            ('xl/media/image1.png', IMAGE),
        ]:
            z.writestr(name, body)

    print('wrote', path, os.path.getsize(path), 'bytes')


if __name__ == '__main__':
    main()
