#!/usr/bin/env python3
"""The seven arms in BODY text -- which answers the wrong question. See `build-footer.py`.

Kept because the correction is the finding: this tree draws a body `NUMPAGES` from its
cache, so every arm here differs by the VALUE as well as by its formatting and the
formatting rule cannot be read out of it. `pages-r164` measured this file and reported a
formatting table that is really a substitution table.

The original docstring follows.

Fixture: does a w:fldSimple.s CACHED RESULT run keep its own w:rPr?

Paragraph style 'Body' states sz=20 (10pt) and no colour.  Each arm puts a field
whose cached result run states sz=40 (20pt) + red, next to a plain run stating the
same rPr as a control.  If the engine honours the cached run the field text is
20pt red like the control; if it discards it the field text is 10pt black.

Writes word/settings.xml (empty) -- without it the importer takes different
OOXML compatibility defaults and the fixture answers a different question.
"""
import os, sys, zipfile

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>'''

SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:rPr><w:sz w:val="20"/><w:szCs w:val="20"/></w:rPr></w:style>
</w:styles>'''

RPR = '<w:rPr><w:color w:val="FF0000"/><w:sz w:val="40"/><w:szCs w:val="40"/></w:rPr>'

def arm(label, xml):
    return ('<w:p><w:r><w:t xml:space="preserve">%s </w:t></w:r>%s</w:p>' % (label, xml))

BODY = "".join([
  # 1. control: a plain run carrying the same rPr
  arm("CTRL",  '<w:r>%s<w:t>XXXX</w:t></w:r>' % RPR),
  # 2. fldSimple NUMPAGES, cached result run carries the rPr
  arm("SIMPLE", '<w:fldSimple w:instr=" NUMPAGES  \\* MERGEFORMAT "><w:r>%s<w:t>9</w:t></w:r></w:fldSimple>' % RPR),
  # 3. complex field NUMPAGES, result run between separate and end carries the rPr
  arm("COMPLEX",
      '<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
      '<w:r><w:instrText xml:space="preserve"> NUMPAGES  \\* MERGEFORMAT </w:instrText></w:r>'
      '<w:r><w:fldChar w:fldCharType="separate"/></w:r>'
      '<w:r>%s<w:t>9</w:t></w:r>'
      '<w:r><w:fldChar w:fldCharType="end"/></w:r>' % RPR),
  # 4. fldSimple with its OWN w:rPr child as well
  arm("SIMPLERPR",
      '<w:fldSimple w:instr=" NUMPAGES  \\* MERGEFORMAT ">%s<w:r>%s<w:t>9</w:t></w:r></w:fldSimple>' % (RPR, RPR)),
  # 5. fldSimple over a field whose value CANNOT change, to separate 'recomputed' from 'fldSimple'
  arm("SIMPLEPAGE", '<w:fldSimple w:instr=" PAGE  \\* MERGEFORMAT "><w:r>%s<w:t>1</w:t></w:r></w:fldSimple>' % RPR),
  # 6. fldSimple WITHOUT \\* MERGEFORMAT
  arm("NOMERGE", '<w:fldSimple w:instr=" NUMPAGES "><w:r>%s<w:t>9</w:t></w:r></w:fldSimple>' % RPR),
  # 7. complex field whose OWN fldChar/instrText runs carry the rPr (CRIF's PAGE shape)
  arm("FIELDRUNRPR",
      '<w:r>%s<w:fldChar w:fldCharType="begin"/></w:r>'
      '<w:r>%s<w:instrText xml:space="preserve"> NUMPAGES  \\* MERGEFORMAT </w:instrText></w:r>'
      '<w:r>%s<w:fldChar w:fldCharType="separate"/></w:r>'
      '<w:r><w:rPr><w:sz w:val="20"/></w:rPr><w:t>9</w:t></w:r>'
      '<w:r>%s<w:fldChar w:fldCharType="end"/></w:r>' % (RPR, RPR, RPR, RPR)),
])

DOC = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>'
 + BODY +
 '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
 '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" w:header="709" w:footer="709" w:gutter="0"/>'
 '</w:sectPr></w:body></w:document>')

out = sys.argv[1]
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
    z.writestr("[Content_Types].xml", CT)
    z.writestr("_rels/.rels", RELS)
    z.writestr("word/_rels/document.xml.rels", DRELS)
    z.writestr("word/settings.xml", SETTINGS)
    z.writestr("word/styles.xml", STYLES)
    z.writestr("word/document.xml", DOC)
print("wrote", out)
