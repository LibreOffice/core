#!/usr/bin/env python3
"""Author `tests/corpus/features/table-style-first-row.docx`.

A three-row table naming a style whose `w:tblStylePr w:type="firstRow"` turns bold on and whose
`lastRow` layer turns italic on, with `w:tblLook` asking for the first row and *not* the last. So
one document carries the rule and its own control: the heading row must come out bold, the final
row must not come out italic, and a run in the heading that says `w:b w:val="0"` outright must
still come out upright.

LibreOffice 26.2.4.2's own PDF of it is the ground truth — `pdffonts` shows
`LiberationSerif-Bold` used for the heading row and nothing italic anywhere.
"""
import os, sys, zipfile

W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main"
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "..",
    "tests", "corpus", "features", "table-style-first-row.docx")

CT = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>'''

STYLES = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="{W}">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/><w:szCs w:val="24"/>
</w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault>
</w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
<w:style w:type="table" w:styleId="BandedHead">
  <w:name w:val="Banded Head"/>
  <w:tblPr><w:tblBorders>
    <w:top w:val="single" w:sz="4" w:space="0" w:color="000000"/>
    <w:left w:val="single" w:sz="4" w:space="0" w:color="000000"/>
    <w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/>
    <w:right w:val="single" w:sz="4" w:space="0" w:color="000000"/>
    <w:insideH w:val="single" w:sz="4" w:space="0" w:color="000000"/>
    <w:insideV w:val="single" w:sz="4" w:space="0" w:color="000000"/>
  </w:tblBorders></w:tblPr>
  <w:tblStylePr w:type="firstRow"><w:rPr><w:b/><w:bCs/></w:rPr></w:tblStylePr>
  <w:tblStylePr w:type="lastRow"><w:rPr><w:i/><w:iCs/></w:rPr></w:tblStylePr>
</w:style>
</w:styles>'''


def cell(text, bold_off=False):
    off = '<w:b w:val="0"/>' if bold_off else ''
    return (f'<w:tc><w:tcPr><w:tcW w:w="4000" w:type="dxa"/></w:tcPr>'
            f'<w:p><w:r><w:rPr>{off}</w:rPr><w:t>{text}</w:t></w:r></w:p></w:tc>')


DOC = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="{W}"><w:body>
<w:tbl>
<w:tblPr><w:tblStyle w:val="BandedHead"/><w:tblW w:w="0" w:type="auto"/>
<w:tblLook w:val="0020" w:firstRow="1" w:lastRow="0" w:firstColumn="0" w:lastColumn="0"
           w:noHBand="1" w:noVBand="1"/></w:tblPr>
<w:tblGrid><w:gridCol w:w="4000"/><w:gridCol w:w="4000"/></w:tblGrid>
<w:tr>{cell("HEADA")}{cell("HEADB", bold_off=True)}</w:tr>
<w:tr>{cell("BODYA")}{cell("BODYB")}</w:tr>
<w:tr>{cell("TAILA")}{cell("TAILB")}</w:tr>
</w:tbl>
<w:p/>
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720"/>
</w:sectPr></w:body></w:document>'''

path = os.path.abspath(OUT)
os.makedirs(os.path.dirname(path), exist_ok=True)
with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
    z.writestr("[Content_Types].xml", CT)
    z.writestr("_rels/.rels", RELS)
    z.writestr("word/_rels/document.xml.rels", DRELS)
    z.writestr("word/styles.xml", STYLES)
    z.writestr("word/document.xml", DOC)
print(path)
