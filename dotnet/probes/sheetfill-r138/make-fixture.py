#!/usr/bin/env python3
"""Build `features/sheet-shape-picture-fill.xlsx`, the O89 fixture.

One worksheet, one `xdr:twoCellAnchor` holding an `xdr:sp` whose own `xdr:spPr`
states `a:blipFill` with `a:stretch` -- a bitmap used as the SHAPE'S INTERIOR,
which is not an `xdr:pic` and which no spreadsheet reader drew.

The corpus's ten instances are all `prstGeom` `rect` + `a:stretch` + `a:srcRect`,
unrotated, so the fixture is that shape. The image is a 2x2 PNG so the file stays
a few hundred bytes; what is under test is whether anything is drawn in the
shape's rectangle at all, not which pixels.
"""
import struct, zlib, zipfile, pathlib, sys

def png(w, h, rgb):
    def chunk(tag, data):
        c = tag + data
        return struct.pack('>I', len(data)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)
    ihdr = struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0)
    raw = b''.join(b'\x00' + bytes(rgb) * w for _ in range(h))
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', ihdr)
            + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))

A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
XDR = 'http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'

PARTS = {
'[Content_Types].xml': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
</Types>''',
'_rels/.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/officeDocument" Target="xl/workbook.xml"/>
</Relationships>''',
'xl/workbook.xml': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="{R}">
<sheets><sheet name="Probe" sheetId="1" r:id="rId1"/></sheets>
</workbook>''',
'xl/_rels/workbook.xml.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="{R}/styles" Target="styles.xml"/>
</Relationships>''',
'xl/styles.xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf/></cellStyleXfs>
<cellXfs count="1"><xf xfId="0"/></cellXfs>
</styleSheet>''',
'xl/worksheets/sheet1.xml': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="{R}">
<sheetData><row r="1"><c r="A1" t="inlineStr"><is><t>probe</t></is></c></row></sheetData>
<drawing r:id="rId1"/>
</worksheet>''',
'xl/worksheets/_rels/sheet1.xml.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/drawing" Target="../drawings/drawing1.xml"/>
</Relationships>''',
'xl/drawings/_rels/drawing1.xml.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/image" Target="../media/image1.png"/>
</Relationships>''',
# The shape: rect geometry, a:blipFill with a:stretch in its OWN spPr, no xdr:pic anywhere.
'xl/drawings/drawing1.xml': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xdr:wsDr xmlns:xdr="{XDR}" xmlns:a="{A}" xmlns:r="{R}">
<xdr:twoCellAnchor editAs="oneCell">
<xdr:from><xdr:col>1</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>1</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>
<xdr:to><xdr:col>4</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>5</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>
<xdr:sp macro="" textlink="">
<xdr:nvSpPr><xdr:cNvPr id="2" name="PictureFilled"/><xdr:cNvSpPr/></xdr:nvSpPr>
<xdr:spPr>
<a:xfrm><a:off x="914400" y="228600"/><a:ext cx="1828800" cy="914400"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:blipFill rotWithShape="1"><a:blip r:embed="rId1"/><a:stretch><a:fillRect/></a:stretch></a:blipFill>
<a:ln><a:noFill/></a:ln>
</xdr:spPr>
<xdr:txBody><a:bodyPr/><a:p><a:endParaRPr lang="en-GB"/></a:p></xdr:txBody>
</xdr:sp>
<xdr:clientData/>
</xdr:twoCellAnchor>
</xdr:wsDr>''',
}

def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1
                       else 'dotnet/tests/corpus/features/sheet-shape-picture-fill.xlsx')
    out.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in PARTS.items():
            z.writestr(name, text)
        z.writestr('xl/media/image1.png', png(2, 2, (0xE0, 0x30, 0x30)))
    print(f'wrote {out} ({out.stat().st_size} bytes)')

if __name__ == '__main__':
    main()
