#!/usr/bin/env python3
"""Build `origins.docx`: three anchored squares, one per horizontal origin, offset nought.

Each square is a different colour so a raster tells them apart without reading operators,
and each sits at a different vertical offset so they cannot overlap. The page is landscape
A4 with 72 pt margins on every side, so the three answers are distinct: a `page` origin puts
a square at 0, and `column` and `margin` both put one at 72.
"""
import zipfile

def anchor(relative, colour, index):
    return f'''<w:r><w:drawing>
   <wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="{index}"
              behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
     <wp:simplePos x="0" y="0"/>
     <wp:positionH relativeFrom="{relative}"><wp:posOffset>0</wp:posOffset></wp:positionH>
     <wp:positionV relativeFrom="paragraph"><wp:posOffset>{index * 400000}</wp:posOffset></wp:positionV>
     <wp:extent cx="457200" cy="228600"/><wp:wrapNone/>
     <wp:docPr id="{index}" name="P{index}"/>
     <a:graphic><a:graphicData><wps:wsp><wps:spPr>
       <a:xfrm><a:off x="0" y="0"/><a:ext cx="457200" cy="228600"/></a:xfrm>
       <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
       <a:solidFill><a:srgbClr val="{colour}"/></a:solidFill>
     </wps:spPr></wps:wsp></a:graphicData></a:graphic>
   </wp:anchor></w:drawing></w:r>'''

BODY = anchor('column', 'FF0000', 1) + anchor('margin', '00FF00', 2) + anchor('page', '0000FF', 3)

DOCUMENT = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
            xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
            xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
            xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
 <w:body><w:p>{BODY}</w:p>
  <w:sectPr>
    <w:pgSz w:w="16838" w:h="11906" w:orient="landscape"/>
    <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
             w:header="720" w:footer="720" w:gutter="0"/>
    <w:cols w:space="720"/>
  </w:sectPr>
 </w:body></w:document>'''

CONTENT_TYPES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
 <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
 <Default Extension="xml" ContentType="application/xml"/>
 <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>'''

RELATIONSHIPS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
 <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

with zipfile.ZipFile('origins.docx', 'w', zipfile.ZIP_DEFLATED) as package:
    package.writestr('[Content_Types].xml', CONTENT_TYPES)
    package.writestr('_rels/.rels', RELATIONSHIPS)
    package.writestr('word/document.xml', DOCUMENT)

print('origins.docx')
