#!/usr/bin/env python3
"""Build DOCX fixtures that separate where an inline drawing's **ink** lands from
where its **text** lands, vertically.

`words-inline-effectextent/make-fixture.py` varies the effect extent and reads the
*gap* between two text lines, which is the line box; `make-x-fixture.py` varies the
horizontal edges and reads columns. Neither reads the shape's own band *and* the run
inside its text box down the page at the same time, which is the one thing the
draw-shape/TextBox disagreement needs.

Each fixture is one portrait page: a 12 pt `TOPLINE`, a paragraph holding one
144 x 50.4 pt inline drawing, a 12 pt `BOTLINE`.

Usage:  python3 makeprobe.py <outdir>
"""
import os, sys, struct, zlib, zipfile

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Default Extension="png" ContentType="image/png"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
      '</Types>')

RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
        '</Relationships>')

DRELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
         '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
         '<Relationship Id="rIdP" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/probe.png"/>'
         '</Relationships>')

RPR = ('<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
       '<w:sz w:val="24"/></w:rPr>')

# A shape with no text box at all: solid black, so its band is unambiguous.
PLAIN_SP = '''<wps:wsp><wps:cNvSpPr/><wps:spPr>
<a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="000000"/></a:solidFill>
<a:ln w="0"><a:noFill/></a:ln>
</wps:spPr><wps:bodyPr/></wps:wsp>'''

# A shape carrying a text box: a light fill so the shape's own band is still readable
# against the page, and a centred `INSIDE` run whose y is the question.
TBX_SP = '''<wps:wsp><wps:cNvSpPr/><wps:spPr>
<a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="C0C0C0"/></a:solidFill>
<a:ln w="0"><a:noFill/></a:ln>
</wps:spPr>
<wps:txbx><w:txbxContent><w:p><w:pPr><w:jc w:val="center"/></w:pPr>
<w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr><w:t>INSIDE</w:t></w:r>
</w:p></w:txbxContent></wps:txbx>
<wps:bodyPr anchor="ctr" lIns="0" tIns="0" rIns="0" bIns="0"/></wps:wsp>'''

# An ellipse, so the *geometry* is read rather than the frame rectangle: a preset whose
# outline is not its own box cannot be matched by moving a rectangle alone.
OVAL_SP = '''<wps:wsp><wps:cNvSpPr/><wps:spPr>
<a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="000000"/></a:solidFill>
<a:ln w="0"><a:noFill/></a:ln>
</wps:spPr><wps:bodyPr/></wps:wsp>'''

# A picture that keeps its shape because it states an `a:effectLst` — the one picture
# case that takes the extent at all, per `words-inline-effectextent/results.md`.
PIC_SP = '''<pic:pic xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture">
<pic:nvPicPr><pic:cNvPr id="2" name="p.png"/><pic:cNvPicPr/></pic:nvPicPr>
<pic:blipFill><a:blip r:embed="rIdP"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>
<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:effectLst><a:outerShdw blurRad="50800" dist="38100" dir="2700000" algn="tl">
<a:srgbClr val="000000"><a:alpha val="40000"/></a:srgbClr></a:outerShdw></a:effectLst>
</pic:spPr></pic:pic>'''

URI = {'wps': 'http://schemas.microsoft.com/office/word/2010/wordprocessingShape',
       'pic': 'http://schemas.openxmlformats.org/drawingml/2006/picture'}

DOC = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<w:body>
<w:p><w:r>{rpr}<w:t>TOPLINE</w:t></w:r></w:p>
<w:p><w:r>{rpr}
<w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="{cx}" cy="{cy}"/>
<wp:effectExtent l="{el}" t="{et}" r="{er}" b="{eb}"/>
<wp:docPr id="1" name="probe"/>
<a:graphic><a:graphicData uri="{uri}">
{sp}</a:graphicData></a:graphic></wp:inline></w:drawing>
</w:r></w:p>
<w:p><w:r>{rpr}<w:t>BOTLINE</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr></w:body></w:document>'''


def png(width=8, height=8, grey=0):
    """A tiny greyscale PNG, written by hand so the fixtures need no image library."""
    raw = b''.join(b'\x00' + bytes([grey]) * width for _ in range(height))

    def chunk(tag, data):
        body = tag + data
        return struct.pack('>I', len(data)) + body + struct.pack('>I', zlib.crc32(body))

    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 0, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(raw))
            + chunk(b'IEND', b''))


def build(path, cx, cy, el, et, er, eb, sp=PLAIN_SP, kind='wps'):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', DOC.format(
            rpr=RPR, sp=sp.format(cx=cx, cy=cy), uri=URI[kind],
            cx=cx, cy=cy, el=el, et=et, er=er, eb=eb))
        if kind == 'pic':
            z.writestr('word/_rels/document.xml.rels', DRELS)
            z.writestr('word/media/probe.png', png())


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    CX, CY = 1828800, 640080          # 144 x 50.4 pt, as the other two generators
    P = lambda n: os.path.join(out, n + '.docx')

    # A plain shape, at the three extents the catalogue uses and at zero.
    for name, e in [('pl-ee0', 0), ('pl-ee27432', 27432), ('pl-ee91440', 91440),
                    ('pl-ee137160', 137160)]:
        build(P(name), CX, CY, e, e, e, e)
    # Which edge the ink follows: the top one alone, and the bottom one alone.
    build(P('pl-t-only'), CX, CY, 0, 137160, 0, 0)
    build(P('pl-b-only'), CX, CY, 0, 0, 0, 137160)

    # The same with a text box, which is where the two halves part company.
    build(P('tb-ee0'),      CX, CY, 0, 0, 0, 0, TBX_SP)
    build(P('tb-ee137160'), CX, CY, 137160, 137160, 137160, 137160, TBX_SP)
    build(P('tb-t-only'),   CX, CY, 0, 137160, 0, 0, TBX_SP)
    build(P('tb-b-only'),   CX, CY, 0, 0, 0, 137160, TBX_SP)

    # A preset whose outline is not its own rectangle.
    build(P('ov-ee0'),      CX, CY, 0, 0, 0, 0, OVAL_SP)
    build(P('ov-t-only'),   CX, CY, 0, 137160, 0, 0, OVAL_SP)

    # A picture that keeps its shape because it declares an effect.
    build(P('px-ee0'),      CX, CY, 0, 0, 0, 0, PIC_SP, 'pic')
    build(P('px-t-only'),   CX, CY, 0, 137160, 0, 0, PIC_SP, 'pic')
    print('built in', out)
