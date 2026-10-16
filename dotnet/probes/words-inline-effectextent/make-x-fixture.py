#!/usr/bin/env python3
"""Build minimal DOCX fixtures that vary a `wp:effectExtent`'s **horizontal**
edges on an inline (`wp:inline`) drawing.

`make-fixture.py` varies only `t`/`b`, which is why the corpus-wide horizontal
defect survived the round that built it: every figure in `results.md` above the
"Horizontally" section was taken from a fixture whose x never moved.

Each fixture is one paragraph reading `LEFT` + drawing + `RIGHT`, so three things
can be read off one page: where the drawing's own ink starts, how much room the
drawing took on the line, and — for the `tbx-*` fixtures, whose shape carries a
`wps:txbx` — where the text *inside* the shape lands.
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

RPR = ('<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
       '<w:sz w:val="24"/></w:rPr>')

# A shape with no text box: solid black, so its ink band is unambiguous.
PLAIN_SP = '''<wps:wsp><wps:cNvSpPr/><wps:spPr>
<a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="000000"/></a:solidFill>
<a:ln w="0"><a:noFill/></a:ln>
</wps:spPr><wps:bodyPr/></wps:wsp>'''

# A shape carrying a text box: a light fill so the shape's own band is still
# readable, and a centred `INSIDE` run whose x is the question.
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

DOC = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<w:body>
<w:p>
<w:r>{rpr}<w:t xml:space="preserve">LEFT </w:t></w:r>
<w:r>{rpr}
<w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="{cx}" cy="{cy}"/>
<wp:effectExtent l="{el}" t="{et}" r="{er}" b="{eb}"/>
<wp:docPr id="1" name="probe"/>
<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
{sp}</a:graphicData></a:graphic></wp:inline></w:drawing>
</w:r>
<w:r>{rpr}<w:t xml:space="preserve"> RIGHT</w:t></w:r>
</w:p>
<w:sectPr><w:pgSz w:w="16838" w:h="11906" w:orient="landscape"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr></w:body></w:document>'''


def build(path, cx, cy, el, et, er, eb, sp=PLAIN_SP):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', DOC.format(
            rpr=RPR, sp=sp.format(cx=cx, cy=cy),
            cx=cx, cy=cy, el=el, et=et, er=er, eb=eb))


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    CX, CY = 1828800, 640080          # 144 x 50.4 pt, as `make-fixture.py`
    E = 137160                        # 10.8 pt, the catalogue's largest extent
    # A plain shape: does the drawn box move right by `l`, and does the line grow by `l+r`?
    build(os.path.join(out, 'x-ee0.docx'),      CX, CY, 0, 0, 0, 0)
    build(os.path.join(out, 'x-l-only.docx'),   CX, CY, E, 0, 0, 0)
    build(os.path.join(out, 'x-r-only.docx'),   CX, CY, 0, 0, E, 0)
    build(os.path.join(out, 'x-ee-all.docx'),   CX, CY, E, E, E, E)
    # The same, with a text box: does the *text* move horizontally, where it does not move
    # vertically? That asymmetry is the whole question.
    build(os.path.join(out, 'tbx-ee0.docx'),    CX, CY, 0, 0, 0, 0, TBX_SP)
    build(os.path.join(out, 'tbx-l-only.docx'), CX, CY, E, 0, 0, 0, TBX_SP)
    build(os.path.join(out, 'tbx-t-only.docx'), CX, CY, 0, E, 0, 0, TBX_SP)
    build(os.path.join(out, 'tbx-ee-all.docx'), CX, CY, E, E, E, E, TBX_SP)
    print('built in', out)
