#!/usr/bin/env python3
"""Probe: what does LibreOffice take (prop-100)% of, for w:line="288" w:lineRule="auto"?

Every group is six paragraphs of one style (contextualSpacing, so the pitch is the
line height plus the proportional gap and nothing else). Only the paragraph mark's
rPr and one interior run's rPr vary between groups.
"""
import zipfile, sys
from pathlib import Path

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>"""
RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""
DRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>"""

STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri" w:cs="Calibri"/>
<w:sz w:val="22"/><w:szCs w:val="22"/></w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault>
</w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
<w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial" w:cs="Arial"/><w:sz w:val="20"/><w:szCs w:val="20"/></w:rPr></w:style>
<w:style w:type="paragraph" w:styleId="P0"><w:name w:val="p0"/><w:basedOn w:val="Normal"/>
<w:pPr><w:spacing w:before="120" w:after="120" w:line="288" w:lineRule="auto"/><w:contextualSpacing/></w:pPr></w:style>
</w:styles>"""

SECT = ('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" '
        'w:header="567" w:footer="567" w:gutter="0"/></w:sectPr>')

def group(label, mark_rpr, run_rpr, n=6, body=None):
    out = [f'<w:p><w:r><w:t>{label}</w:t></w:r></w:p>']
    for i in range(n):
        ppr = f'<w:pPr><w:pStyle w:val="P0"/>{mark_rpr}</w:pPr>'
        if body is not None:
            extra = body.replace('@RPR@', run_rpr)
        else:
            extra = f'<w:r>{run_rpr}<w:t xml:space="preserve"> x </w:t></w:r>' if run_rpr else ''
        out.append(f'<w:p>{ppr}<w:r><w:t>{label}{i}</w:t></w:r>{extra}</w:p>')
    return ''.join(out)

CAL11 = '<w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="22"/></w:rPr>'
CAL22 = '<w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="44"/></w:rPr>'
ARI11 = '<w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="22"/></w:rPr>'
ARI20 = '<w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="40"/></w:rPr>'

body = ''.join([
    group('A', '', ''),                 # bare: mark inherits P0 (Arial 10)
    group('B', CAL11, ''),              # paragraph mark rPr = Calibri 11
    group('C', ARI11, ''),              # paragraph mark rPr = Arial 11
    group('D', ARI20, ''),              # paragraph mark rPr = Arial 20
    group('E', '', CAL11),              # a Calibri 11 run in the text
    group('F', '', CAL11, body='<w:r>@RPR@<w:tab/></w:r><w:r><w:t>end</w:t></w:r>'),
    group('G', '', CAL11, body='<w:r>@RPR@<w:t xml:space="preserve"> </w:t></w:r><w:r><w:t>end</w:t></w:r>'),
    group('H', '', CAL22, body='<w:r>@RPR@<w:tab/></w:r><w:r><w:t>end</w:t></w:r>'),
    group('I', '', ARI20, body='<w:r>@RPR@<w:tab/></w:r><w:r><w:t>end</w:t></w:r>'),
])

doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
       f'<w:body>{body}{SECT}</w:body></w:document>')

path = Path(sys.argv[1])
with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/_rels/document.xml.rels', DRELS)
    z.writestr('word/styles.xml', STYLES)
    z.writestr('word/document.xml', doc)
print(path)
