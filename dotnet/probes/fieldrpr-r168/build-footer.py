#!/usr/bin/env python3
"""The same seven field arms, in a FOOTER — where the value is recomputed at paint time.

`build.py`'s body-text fixture cannot answer the formatting question, because this tree
draws a body `NUMPAGES` from its cache and the formatting of a value it never computed is
not a rule about formatting. A footer is where both engines recompute, so the only thing
left to differ is which run's `w:rPr` the recomputed value takes.

`word/settings.xml` is written even though it is empty (`office-corpus/SKILL.md`).
"""
import os, sys, zipfile

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
<Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>
</Types>'''
RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>'''
DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" Target="footer1.xml"/></Relationships>'''
SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>'''
STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:rPr><w:sz w:val="20"/><w:szCs w:val="20"/></w:rPr></w:style>
</w:styles>'''

RPR = '<w:rPr><w:color w:val="FF0000"/><w:sz w:val="40"/><w:szCs w:val="40"/></w:rPr>'
NS = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'


def arm(label, xml):
    return f'<w:p><w:r><w:t xml:space="preserve">{label} </w:t></w:r>{xml}</w:p>'


ARMS = [
    ("CTRL", f'<w:r>{RPR}<w:t>XXXX</w:t></w:r>'),
    ("SIMPLE",
     f'<w:fldSimple w:instr=" PAGE  \\* MERGEFORMAT "><w:r>{RPR}<w:t>9</w:t></w:r></w:fldSimple>'),
    ("COMPLEX",
     '<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
     '<w:r><w:instrText xml:space="preserve"> PAGE  \\* MERGEFORMAT </w:instrText></w:r>'
     '<w:r><w:fldChar w:fldCharType="separate"/></w:r>'
     f'<w:r>{RPR}<w:t>9</w:t></w:r>'
     '<w:r><w:fldChar w:fldCharType="end"/></w:r>'),
    ("SIMPLERPR",
     f'<w:fldSimple w:instr=" PAGE  \\* MERGEFORMAT ">{RPR}<w:r>{RPR}<w:t>9</w:t></w:r></w:fldSimple>'),
    ("NOMERGE",
     f'<w:fldSimple w:instr=" PAGE "><w:r>{RPR}<w:t>9</w:t></w:r></w:fldSimple>'),
    ("FIELDRUNRPR",
     f'<w:r>{RPR}<w:fldChar w:fldCharType="begin"/></w:r>'
     f'<w:r>{RPR}<w:instrText xml:space="preserve"> PAGE  \\* MERGEFORMAT </w:instrText></w:r>'
     f'<w:r>{RPR}<w:fldChar w:fldCharType="separate"/></w:r>'
     '<w:r><w:rPr><w:sz w:val="20"/></w:rPr><w:t>9</w:t></w:r>'
     f'<w:r>{RPR}<w:fldChar w:fldCharType="end"/></w:r>'),
    ("NOSEPARATOR",
     '<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
     f'<w:r>{RPR}<w:instrText xml:space="preserve"> PAGE  \\* MERGEFORMAT </w:instrText></w:r>'
     '<w:r><w:fldChar w:fldCharType="end"/></w:r>'),
]

FOOTER = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:ftr {NS}>'
          + "".join(arm(label, xml) for label, xml in ARMS) + '</w:ftr>')

DOC = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {NS}><w:body>'
       '<w:p><w:r><w:t>BODY</w:t></w:r></w:p>'
       '<w:sectPr><w:footerReference w:type="default" r:id="rId3"'
       ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"/>'
       '<w:pgSz w:w="11906" w:h="16838"/>'
       '<w:pgMar w:top="1134" w:right="1134" w:bottom="4000" w:left="1134"'
       ' w:header="709" w:footer="709" w:gutter="0"/></w:sectPr></w:body></w:document>')

out = sys.argv[1] if len(sys.argv) > 1 else "fldfooter.docx"
if os.path.exists(out):
    os.remove(out)
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
    z.writestr("[Content_Types].xml", CT)
    z.writestr("_rels/.rels", RELS)
    z.writestr("word/_rels/document.xml.rels", DRELS)
    z.writestr("word/settings.xml", SETTINGS)
    z.writestr("word/styles.xml", STYLES)
    z.writestr("word/footer1.xml", FOOTER)
    z.writestr("word/document.xml", DOC)
print("wrote", out)
