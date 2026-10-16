#!/usr/bin/env python3
"""Build minimal DOCX fixtures holding one inline (`wp:inline`) DrawingML shape
whose `wp:effectExtent` is varied, so the reference's treatment of that element
can be measured against a fixed pair of text lines above and below it."""
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

DOC = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<w:body>
<w:p><w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr><w:t>TOPLINE</w:t></w:r></w:p>
<w:p><w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>
<w:drawing><wp:inline distT="{dT}" distB="{dB}" distL="{dL}" distR="{dR}">
<wp:extent cx="{cx}" cy="{cy}"/>
<wp:effectExtent l="{el}" t="{et}" r="{er}" b="{eb}"/>
<wp:docPr id="1" name="probe"/>
<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<wps:wsp><wps:cNvSpPr/><wps:spPr>
<a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="000000"/></a:solidFill>
<a:ln w="0"><a:noFill/></a:ln>
</wps:spPr>
<wps:bodyPr/></wps:wsp></a:graphicData></a:graphic></wp:inline></w:drawing>
</w:r></w:p>
<w:p><w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr><w:t>BOTLINE</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr></w:body></w:document>'''


def build(path, cx, cy, el, et, er, eb, dL=0, dT=0, dR=0, dB=0):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', DOC.format(
            cx=cx, cy=cy, el=el, et=et, er=er, eb=eb, dL=dL, dT=dT, dR=dR, dB=dB))


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    CY = 640080          # 50.4 pt, the catalogue's commonest shape height
    CX = 1828800         # 144 pt
    # The three effect extents the catalogue actually uses, plus zero as the control.
    for name, e in [('ee0', 0), ('ee27432', 27432), ('ee91440', 91440), ('ee137160', 137160)]:
        build(os.path.join(out, f'{name}.docx'), CX, CY, e, e, e, e)
    # Asymmetric, to see which side each of t/b lands on.
    build(os.path.join(out, 'ee-t-only.docx'), CX, CY, 0, 137160, 0, 0)
    build(os.path.join(out, 'ee-b-only.docx'), CX, CY, 0, 0, 0, 137160)
    # dist* alone, to check it is additive with the effect extent rather than an alternative.
    build(os.path.join(out, 'dist-only.docx'), CX, CY, 0, 0, 0, 0, dT=137160, dB=137160)
    print('built in', out)
