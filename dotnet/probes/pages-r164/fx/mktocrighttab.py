#!/usr/bin/env python3
"""Fixture: a right-aligned dot-leader tab stop that sits BEYOND the paragraph's
right indent -- the shape of every `TOC2` entry in 02_mcar.

Text column is 9360 twips (468 pt) and the right tab stop is at 9360, so the stop
sits exactly on the column's right edge.  `w:ind w:right=R` pulls the wrap boundary
R twips to the left of it.  Each arm keeps the stop and varies the title's length,
so the sweep finds the title width at which each engine stops fitting the page
number on the entry's first line.

Writes word/settings.xml (empty): without it the importer takes different OOXML
compatibility defaults and the fixture answers a different question.
"""
import sys, zipfile

CT='''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>'''
RELS='''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>'''
DRELS='''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/></Relationships>'''
SETTINGS='''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>'''
STYLES='''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="DejaVu Sans" w:hAnsi="DejaVu Sans"/><w:sz w:val="22"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
</w:styles>'''

def para(right, title):
    return ('<w:p><w:pPr>'
      '<w:tabs><w:tab w:val="left" w:pos="990"/><w:tab w:val="right" w:leader="dot" w:pos="9360"/></w:tabs>'
      '<w:spacing w:after="0"/>'
      '<w:ind w:left="720" w:right="%d" w:hanging="720"/>'
      '<w:rPr><w:b/></w:rPr></w:pPr>'
      '<w:r><w:rPr><w:b/></w:rPr><w:t>R%d</w:t></w:r>'
      '<w:r><w:rPr><w:b/></w:rPr><w:tab/></w:r>'
      '<w:r><w:rPr><w:b/></w:rPr><w:t>%s</w:t></w:r>'
      '<w:r><w:rPr><w:b/></w:rPr><w:tab/></w:r>'
      '<w:r><w:rPr><w:b/></w:rPr><w:t>2-123</w:t></w:r>'
      '</w:p>') % (right, right, title)

body=[]
for right in (0, 720, 994):
    for n in range(40, 76, 2):
        body.append(para(right, "A"*n))
DOC=('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>'
 + "".join(body) +
 '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
 '<w:pgMar w:top="1080" w:right="1440" w:bottom="1080" w:left="1440" w:header="432" w:footer="432"/>'
 '</w:sectPr></w:body></w:document>')
out=sys.argv[1]
with zipfile.ZipFile(out,"w",zipfile.ZIP_DEFLATED) as z:
    z.writestr("[Content_Types].xml",CT); z.writestr("_rels/.rels",RELS)
    z.writestr("word/_rels/document.xml.rels",DRELS)
    z.writestr("word/settings.xml",SETTINGS); z.writestr("word/styles.xml",STYLES)
    z.writestr("word/document.xml",DOC)
print("wrote",out)
