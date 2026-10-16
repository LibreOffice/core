#!/usr/bin/env python3
"""Probe: how tall is an empty header paragraph with proportional line spacing?

One header, one body line. Only the empty paragraph's w:line and its paragraph mark's
w:sz vary, so the body's first baseline reports the header band's height directly.
"""
import zipfile, sys
from pathlib import Path

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
</Types>"""
RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""
DRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header" Target="header1.xml"/>
</Relationships>"""
STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Arial" w:hAnsi="Arial" w:cs="Arial"/><w:sz w:val="20"/><w:szCs w:val="20"/></w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault>
</w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
</w:styles>"""

HDR = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:hdr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:p><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/></w:pPr><w:r><w:t>HEAD</w:t></w:r></w:p>
<w:p><w:pPr><w:spacing w:after="0" w:line="@LINE@" w:lineRule="auto"/><w:rPr>@MARK@</w:rPr></w:pPr>@FILL@</w:p>
<w:p><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/></w:pPr><w:r><w:t>TAIL</w:t></w:r></w:p>
</w:hdr>"""

DOC = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<w:body><w:p><w:r><w:t>BODY</w:t></w:r></w:p>
<w:sectPr><w:headerReference w:type="default" r:id="rId2"/>
<w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="567" w:bottom="720" w:left="1134" w:header="709" w:footer="709" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""

def build(path, line, mark, fill=''):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/_rels/document.xml.rels', DRELS)
        z.writestr('word/styles.xml', STYLES)
        z.writestr('word/header1.xml', HDR.replace('@LINE@', str(line)).replace('@MARK@', mark).replace('@FILL@', fill))
        z.writestr('word/document.xml', DOC)

out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
cases = {
    'h240-sz20': (240, ''),
    'h480-sz20': (480, ''),
    'h240-sz24': (240, '<w:b/><w:sz w:val="24"/>'),
    'h480-sz24': (480, '<w:b/><w:sz w:val="24"/>'),
    'h360-sz24': (360, '<w:b/><w:sz w:val="24"/>'),
    'h480-sz40': (480, '<w:sz w:val="40"/>'),
}
cases['t240-sz24'] = (240, '<w:b/><w:sz w:val="24"/>')
cases['t480-sz24'] = (480, '<w:b/><w:sz w:val="24"/>')
for name, (line, mark) in cases.items():
    build(out / f'{name}.docx', line, mark, fill='<w:r><w:rPr><w:b/><w:sz w:val="24"/></w:rPr><w:t>MID</w:t></w:r>' if name.startswith('t') else '')
    print(name)
