#!/usr/bin/env python3
"""A three-section DOCX whose middle section is continuous and two-column, for conversion to .odt.

Converted by 26.2.4.2 the middle section becomes a `text:section` carrying `style:columns`, which is
the object every one of the corpus's columned .odt states and the one this round reads.  Authored as
a DOCX and converted rather than hand-written as flat ODF, for the reason `dotnet/CLAUDE.md` records
about hand-built ODF probes: LibreOffice's own writers are the only source of a realistic one.

Every paragraph of the columned section opens with a unique marker so a test can say which column and
which page it landed in.
"""
import zipfile, sys

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

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

def para(text, sect=''):
    return f'<w:p><w:pPr>{sect}</w:pPr><w:r><w:t xml:space="preserve">{text}</w:t></w:r></w:p>'

def sectpr(cols):
    c = f'<w:cols w:num="{cols}" w:space="720"/>' if cols > 1 else '<w:cols w:space="720"/>'
    return ('<w:sectPr><w:type w:val="continuous"/><w:pgSz w:w="12240" w:h="15840"/>'
            '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
            f' w:header="720" w:footer="720" w:gutter="0"/>{c}</w:sectPr>')

LOREM = ('lorem ipsum dolor sit amet consectetuer adipiscing elit sed diam nonummy nibh '
         'euismod tincidunt ut laoreet dolore magna aliquam erat volutpat')

parts = [para('ONEBEFORE the first section is a single column'),
         para(f'ONEBODY {LOREM}', sectpr(1))]
for i in range(1, 15):
    parts.append(para(f'PARA{i:02d} {LOREM}'))
parts.append(para('TWOEND the columned section ends here', sectpr(2)))
parts.append(para('THREEAFTER back to one column'))
parts.append(para(f'THREEBODY {LOREM}'))

doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       f'<w:document {W}><w:body>{"".join(parts)}{sectpr(1)}</w:body></w:document>')

with zipfile.ZipFile(sys.argv[1], 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/document.xml', doc)
print('wrote', sys.argv[1])
