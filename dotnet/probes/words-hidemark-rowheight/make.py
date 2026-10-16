"""Build four one-table DOCX probes that differ only in `w:hideMark` and cell content.

The question is LibreOffice's rule at `sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx`
:1157 — when every cell of a row carries `w:hideMark` *and* the row is empty, the row's height is
forced from a minimum to exactly `w:trHeight`. What counts as "empty" is `lcl_emptyRow` comparing
each cell's start and end text ranges, which the corpus's graph-paper templates make load-bearing:
their cells hold a single no-break space.

Each table is 10 rows of 3 cells, `w:trHeight w:val="180"` (9 pt) with no `w:hRule`, so the
declared height is well below the ~13.7 pt an 11 pt Calibri line needs. A fixed row draws 9 pt; an
at-least row draws 13.7.
"""
import zipfile, os

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>"""

SETTINGS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>"""

STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri" w:cs="Calibri"/><w:sz w:val="22"/><w:szCs w:val="22"/>
</w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault>
</w:docDefaults></w:styles>"""

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

def cell(hide, text):
    mark = "<w:hideMark/>" if hide else ""
    run = "" if text is None else (
        f'<w:r><w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/></w:rPr>'
        f'<w:t xml:space="preserve">{text}</w:t></w:r>')
    return ('<w:tc><w:tcPr><w:tcW w:w="1200" w:type="dxa"/><w:noWrap/>'
            f'{mark}</w:tcPr>'
            '<w:p><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/></w:pPr>'
            f'{run}</w:p></w:tc>')

def document(hide, text):
    rows = "".join(
        '<w:tr><w:trPr><w:trHeight w:val="180"/></w:trPr>'
        + "".join(cell(hide, text) for _ in range(3))
        + "</w:tr>" for _ in range(10))
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {W}><w:body>
<w:tbl><w:tblPr><w:tblW w:w="3600" w:type="dxa"/>
<w:tblBorders>
<w:top w:val="single" w:sz="4" w:space="0" w:color="000000"/>
<w:left w:val="single" w:sz="4" w:space="0" w:color="000000"/>
<w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/>
<w:right w:val="single" w:sz="4" w:space="0" w:color="000000"/>
<w:insideH w:val="single" w:sz="4" w:space="0" w:color="000000"/>
<w:insideV w:val="single" w:sz="4" w:space="0" w:color="000000"/>
</w:tblBorders></w:tblPr>
<w:tblGrid><w:gridCol w:w="1200"/><w:gridCol w:w="1200"/><w:gridCol w:w="1200"/></w:tblGrid>
{rows}</w:tbl>
<w:p/>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" w:header="709" w:footer="709" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""

CASES = {
    "hidemark-empty":  (True,  None),
    "hidemark-nbsp":   (True,  " "),
    "hidemark-text":   (True,  "x"),
    "plain-nbsp":      (False, " "),
    "plain-empty":     (False, None),
}

os.makedirs("out", exist_ok=True)
for name, (hide, text) in CASES.items():
    with zipfile.ZipFile(f"out/{name}.docx", "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/settings.xml", SETTINGS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", document(hide, text))
    print("out/" + name + ".docx")
