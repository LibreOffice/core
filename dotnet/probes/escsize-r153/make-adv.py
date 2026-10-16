#!/usr/bin/env python3
"""Fixture A -- read the escaped size out of an ADVANCE, not out of a `Tf`.

Round 152 settled the superscript-size rule by reading `span['size']` out of the PDF, which is the
`Tf` operator.  That channel cannot separate "the layout quantised the size" from "the PDF writer
quantised the number it printed", and the three candidate rules -- truncate to a twip, round to a
twip, round to a tenth of a point -- agree at 8 pt, which is the only size the independent
advance channel in that round was run at.

This fixture reads the size out of an advance instead.  Each arm is a RIGHT-ALIGNED line holding a
label and then N copies of one superscript digit, so the line's right edge is the margin on both
sides and the superscript span's own left edge is `margin - N*advance - sidebearing`.  Two arms per
size differ only in N, so differencing them cancels the label, the margin and the side bearing and
leaves exactly (N2-N1) advances of one glyph at the escaped size.

    make-adv.py <out.docx>
"""
import sys, zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
FACE = 'Liberation Sans'
DIGIT = '0'
N1, N2 = 2, 42

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

# An empty settings part: without one an authored DOCX does not get LibreOffice's OOXML
# compatibility defaults (office-corpus skill).
SETTINGS = f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:settings xmlns:w="{W}"/>'

STYLES = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="{W}">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="{FACE}" w:eastAsia="{FACE}" w:hAnsi="{FACE}" w:cs="{FACE}"/><w:sz w:val="20"/><w:szCs w:val="20"/><w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:rPr><w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/></w:rPr></w:style>
</w:styles>'''


def arm(hp, n):
    rpr = (f'<w:rPr><w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}" w:cs="{FACE}"/>'
           f'<w:sz w:val="{hp}"/><w:szCs w:val="{hp}"/><w:vertAlign w:val="superscript"/></w:rPr>')
    lab = (f'<w:rPr><w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/><w:sz w:val="18"/></w:rPr>')
    return ('<w:p><w:pPr><w:jc w:val="right"/></w:pPr>'
            f'<w:r>{lab}<w:t xml:space="preserve">ARM H{hp} N{n} </w:t></w:r>'
            f'<w:r>{rpr}<w:t>{DIGIT * n}</w:t></w:r></w:p>')


HPS = list(range(8, 41))          # 4.0 pt .. 20.0 pt in half-points
body = []
for hp in HPS:
    body.append(arm(hp, N1))
    body.append(arm(hp, N2))

doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:document xmlns:w="{W}"><w:body>'
       + ''.join(body)
       + '<w:sectPr><w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/>'
         '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0" w:gutter="0"/>'
         '</w:sectPr></w:body></w:document>')

out = sys.argv[1]
with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/_rels/document.xml.rels', DRELS)
    z.writestr('word/styles.xml', STYLES)
    z.writestr('word/settings.xml', SETTINGS)
    z.writestr('word/document.xml', doc)
print(f'{out}: {len(body)} arms, {len(HPS)} sizes, N {N1}/{N2}')
