#!/usr/bin/env python3
"""Build one-page DOCX fixtures holding one *rotated* inline drawing, so the room it
takes on its line and the rectangle its ink lands in can both be read off one page.

`probes/words-inline-effectextent/` established that a rotated `wp:inline` grows its
line by 46.25 pt whether or not it states a `wp:effectExtent`, and left the cause as
"the rotated bounding box, which we do not size". These fixtures measure that box.

A black rectangle is used rather than a picture because its ink *is* its snap
rectangle: the bounding box of the drawn pixels is exactly the rotated rectangle, with
no white margin to guess at.
"""
import os, sys, zipfile

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

# Grey rather than black, so the rectangle's ink can be separated from the text's by
# threshold alone: `measure.py` counts a pixel as the drawing's only below 64, and no
# antialiased edge of a 0x909090 glyph reaches that.
RPR = ('<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
       '<w:sz w:val="24"/><w:color w:val="909090"/></w:rPr>')

DOC = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<w:body>
<w:p><w:r>{rpr}<w:t>TOPLINE</w:t></w:r></w:p>
<w:p>
<w:r>{rpr}<w:t xml:space="preserve">LEFT </w:t></w:r>
<w:r>{rpr}
<w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="{cx}" cy="{cy}"/>
<wp:effectExtent l="{e}" t="{e}" r="{e}" b="{e}"/>
<wp:docPr id="1" name="probe"/>
<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<wps:wsp><wps:cNvSpPr/><wps:spPr>
<a:xfrm{rot}><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="000000"/></a:solidFill>
<a:ln w="0"><a:noFill/></a:ln>
</wps:spPr><wps:bodyPr/></wps:wsp></a:graphicData></a:graphic></wp:inline></w:drawing>
</w:r>
<w:r>{rpr}<w:t xml:space="preserve"> RIGHT</w:t></w:r>
</w:p>
<w:p><w:r>{rpr}<w:t>BOTLINE</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="16838" w:h="11906" w:orient="landscape"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr></w:body></w:document>'''


def build(path, cx, cy, degrees, extent=0):
    rot = '' if degrees == 0 else f' rot="{int(round(degrees * 60000))}"'
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml',
                   DOC.format(rpr=RPR, cx=cx, cy=cy, rot=rot, e=extent))


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    CX, CY = 1828800, 640080          # 144 x 50.4 pt, as the effect-extent probe
    for degrees in (0, 20, 45, 90, 135, 315):
        build(os.path.join(out, f'rot{degrees:03d}.docx'), CX, CY, degrees)
    # The same angle with an extent, to re-check that none of the growth is the extent.
    build(os.path.join(out, 'rot020-ee.docx'), CX, CY, 20, extent=137160)
    # A square, where the rotated box is the same width and height at every angle and a
    # width/height swap cannot be told from a bounding box.
    build(os.path.join(out, 'sq-rot020.docx'), 1828800, 1828800, 20)
    print('built in', out)
