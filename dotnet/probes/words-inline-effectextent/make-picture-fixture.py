#!/usr/bin/env python3
"""Build DOCX fixtures whose inline drawing is a PICTURE rather than a
`wps:wsp` shape, with the same `wp:effectExtent` the shape fixtures use.

`GraphicImport.cxx`:805-881 turns a plain picture into a Writer graphic object
(`bUseShape = !m_xGraphicObject.is()`) and disposes the shape, which skips the
whole `if (m_xShape.is())` block -- and with it the effect-extent folding at
:1036-1055. The conversion is refused when the picture is rotated or carries
DrawingML effects, so those stay shapes. These fixtures measure all three."""
import os, struct, sys, zlib, zipfile

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
      'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture"')

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rIdI" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/probe.png"/>
</Relationships>'''

DOC = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document ' + NS + '''><w:body>
<w:p><w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr><w:t>TOPLINE</w:t></w:r></w:p>
<w:p><w:r><w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="{cx}" cy="{cy}"/>
<wp:effectExtent l="{el}" t="{et}" r="{er}" b="{eb}"/>
<wp:docPr id="1" name="probe"/>
<a:graphic><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">
<pic:pic><pic:nvPicPr><pic:cNvPr id="1" name="probe"/><pic:cNvPicPr/></pic:nvPicPr>
<pic:blipFill><a:blip r:embed="rIdI"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>
<pic:spPr><a:xfrm{rot}><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>{effects}</pic:spPr>
</pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>
<w:p><w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr><w:t>BOTLINE</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr></w:body></w:document>'''

SHADOW = ('<a:effectLst><a:outerShdw blurRad="50800" dist="38100" dir="2700000">'
          '<a:srgbClr val="000000"><a:alpha val="40000"/></a:srgbClr></a:outerShdw></a:effectLst>')


def png(w=8, h=8):
    """A solid black PNG, written by hand so the probe needs no image library."""
    raw = b''.join(b'\x00' + b'\x00\x00\x00' * w for _ in range(h))
    def chunk(tag, data):
        c = tag + data
        return struct.pack('>I', len(data)) + c + struct.pack('>I', zlib.crc32(c))
    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(raw))
            + chunk(b'IEND', b''))


def build(path, cx, cy, el, et, er, eb, rot='', effects=''):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/_rels/document.xml.rels', DRELS)
        z.writestr('word/media/probe.png', png())
        z.writestr('word/document.xml', DOC.format(
            cx=cx, cy=cy, el=el, et=et, er=er, eb=eb, rot=rot, effects=effects))


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    CX, CY = 1828800, 640080          # 144 x 50.4 pt, as the shape fixtures
    build(os.path.join(out, 'pic-ee0.docx'), CX, CY, 0, 0, 0, 0)
    build(os.path.join(out, 'pic-ee137160.docx'), CX, CY, 137160, 137160, 137160, 137160)
    # gpp-pr's own asymmetric extent: l 1.5, t 1.5, r 1.7, b 1.85 pt.
    build(os.path.join(out, 'pic-gpp.docx'), CX, CY, 19050, 19050, 21590, 23495)
    # A rotated picture is refused the graphic-object conversion (fdo#70457) and stays a shape.
    build(os.path.join(out, 'pic-ee137160-rot.docx'), CX, CY, 137160, 137160, 137160, 137160,
          rot=' rot="1200000"')
    # So is one carrying DrawingML effects, which land in the grab bag as EffectProperties.
    build(os.path.join(out, 'pic-ee137160-shadow.docx'), CX, CY, 137160, 137160, 137160, 137160,
          effects=SHADOW)
    print('built in', out)
