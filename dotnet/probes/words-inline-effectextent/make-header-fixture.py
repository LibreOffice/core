#!/usr/bin/env python3
"""Build DOCX fixtures holding one inline shape in a HEADER, varying its
`wp:effectExtent`, so the reference's treatment of the header case can be
measured separately from the body case `make-fixture.py` covers.

Motivated by `TE.CAO.00125 ... OJT Logbook.docx`, whose only effect extent is a
0.75 pt bottom edge on a 42.75 pt header logo."""
import os, sys, zipfile

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rIdH" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header" Target="header1.xml"/>
</Relationships>'''

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
      'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape"')

SHAPE = '''<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr><w:r>
<w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="{cx}" cy="{cy}"/>
<wp:effectExtent l="{el}" t="{et}" r="{er}" b="{eb}"/>
<wp:docPr id="1" name="probe"/>
<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<wps:wsp><wps:cNvSpPr/><wps:spPr>
<a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="000000"/></a:solidFill>
<a:ln w="0"><a:noFill/></a:ln>
</wps:spPr><wps:bodyPr/></wps:wsp></a:graphicData></a:graphic></wp:inline></w:drawing>
</w:r></w:p>'''

HDR = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:hdr ' + NS + '>{shape}</w:hdr>'

DOC = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document ' + NS + '><w:body>'
       '<w:p><w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
       '<w:sz w:val="24"/></w:rPr><w:t>BODYLINE</w:t></w:r></w:p>'
       '<w:sectPr><w:headerReference w:type="default" r:id="rIdH" '
       'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"/>'
       '<w:pgSz w:w="11906" w:h="16838"/>'
       '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" '
       'w:header="708" w:footer="708" w:gutter="0"/>'
       '</w:sectPr></w:body></w:document>')


def build(path, cx, cy, el, et, er, eb):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/_rels/document.xml.rels', DRELS)
        z.writestr('word/document.xml', DOC)
        z.writestr('word/header1.xml', HDR.format(
            shape=SHAPE.format(cx=cx, cy=cy, el=el, et=et, er=er, eb=eb)))


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    CY = 542925          # 42.75 pt, the OJT Logbook's header logo
    CX = 542925
    build(os.path.join(out, 'hdr-ee0.docx'), CX, CY, 0, 0, 0, 0)
    # The logbook's own shape: a 0.75 pt right and bottom edge and nothing else.
    build(os.path.join(out, 'hdr-ee9525-rb.docx'), CX, CY, 0, 0, 9525, 9525)
    # Larger, so the answer cannot hide inside a rounding step.
    build(os.path.join(out, 'hdr-ee137160-b.docx'), CX, CY, 0, 0, 0, 137160)
    build(os.path.join(out, 'hdr-ee137160-all.docx'), CX, CY, 137160, 137160, 137160, 137160)
    print('built in', out)
