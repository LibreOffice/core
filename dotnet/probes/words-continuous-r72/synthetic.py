#!/usr/bin/env python3
"""The synthetic `ContinuousSectionGeometryTests` builds, as a real package, plus the two
variants that separate the two halves of the rule.

  plain        a continuous second section naming no furniture and holding no page break
  hdr          the same, naming a default header of its own
  hdr-break    the same again, with a hard page break inside the section

Usage: synthetic.py <outdir>
"""
import sys, zipfile
from pathlib import Path

CT = '''<?xml version="1.0" encoding="UTF-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
<Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
</Types>'''

ROOT_RELS = '''<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Target="word/document.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
</Relationships>'''

DOC_RELS = '''<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Target="settings.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
<Relationship Id="rId2" Target="header1.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header"/>
</Relationships>'''

SETTINGS = ('<?xml version="1.0" encoding="UTF-8"?>'
            '<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>')

HEADER = ('<?xml version="1.0" encoding="UTF-8"?>'
          '<w:hdr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
          '<w:p><w:r><w:t>SECOND SECTION RUNNING HEAD</w:t></w:r></w:p></w:hdr>')


def document(header, hard_break):
    filler = ''.join(
        '<w:p>%s<w:r><w:t>Line %d of the continuous section&#8217;s body text.</w:t></w:r></w:p>'
        % ('<w:pPr><w:pageBreakBefore/></w:pPr>' if hard_break and i == 40 else '', i)
        for i in range(90))
    hdr = '<w:headerReference w:type="default" r:id="rId2"/>' if header else ''
    return ('<?xml version="1.0" encoding="UTF-8"?>'
            '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
            ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
            '<w:body>'
            '<w:p><w:pPr><w:sectPr>'
            '<w:pgSz w:w="12240" w:h="15840"/>'
            '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720"'
            ' w:header="720" w:footer="720" w:gutter="0"/>'
            '</w:sectPr></w:pPr><w:r><w:t>Title section</w:t></w:r></w:p>'
            + filler +
            '<w:sectPr>' + hdr + '<w:type w:val="continuous"/>'
            '<w:pgSz w:w="12240" w:h="15840"/>'
            '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
            ' w:header="1440" w:footer="720" w:gutter="0"/>'
            '</w:sectPr></w:body></w:document>')


def build(path, header, hard_break):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('word/_rels/document.xml.rels', DOC_RELS)
        z.writestr('word/settings.xml', SETTINGS)
        z.writestr('word/header1.xml', HEADER)
        z.writestr('word/document.xml', document(header, hard_break))
    print(path.name)


def main():
    out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
    build(out / 'plain.docx', False, False)
    build(out / 'hdr.docx', True, False)
    build(out / 'hdr-break.docx', True, True)


if __name__ == '__main__':
    main()
