#!/usr/bin/env python3
"""A minimal DOCX with one page-anchored floating shape, to ask where `relativeFrom="page"` counts from.

One shape, one paragraph, a 1 inch top margin, and word/settings.xml present -- without that part
LibreOffice takes a different set of OOXML compatibility defaults and the fixture answers a
different question (see the office-corpus skill).
"""
import sys, zipfile

TOP = sys.argv[1] if len(sys.argv) > 1 else '1440'      # twips
OFFSET = sys.argv[2] if len(sys.argv) > 2 else '0'      # EMU
OUT = sys.argv[3] if len(sys.argv) > 3 else 'page0.docx'

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>'''

SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>
</w:settings>'''

DOC = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<w:body>
<w:p><w:r><w:drawing>
<wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="1" behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
<wp:simplePos x="0" y="0"/>
<wp:positionH relativeFrom="page"><wp:posOffset>914400</wp:posOffset></wp:positionH>
<wp:positionV relativeFrom="page"><wp:posOffset>{OFFSET}</wp:posOffset></wp:positionV>
<wp:extent cx="914400" cy="914400"/>
<wp:effectExtent l="0" t="0" r="0" b="0"/>
<wp:wrapNone/>
<wp:docPr id="1" name="Box"/>
<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<wps:wsp>
<wps:cNvSpPr/>
<wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="914400" cy="914400"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="FF0000"/></a:solidFill>
<a:ln><a:noFill/></a:ln></wps:spPr>
<wps:bodyPr/>
</wps:wsp>
</a:graphicData></a:graphic>
</wp:anchor>
</w:drawing></w:r></w:p>
<w:p><w:r><w:t>Body.</w:t></w:r></w:p>
<w:sectPr>
<w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="{TOP}" w:right="1440" w:bottom="1440" w:left="1440" w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr>
</w:body></w:document>'''

z = zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED)
z.writestr('[Content_Types].xml', CT)
z.writestr('_rels/.rels', RELS)
z.writestr('word/_rels/document.xml.rels', DRELS)
z.writestr('word/settings.xml', SETTINGS)
z.writestr('word/document.xml', DOC)
z.close()
print('wrote', OUT)
