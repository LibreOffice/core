#!/usr/bin/env python3
"""Author the adjacent-format probes: a .docx hyperlink and an .xlsx cell hyperlink.

The question each answers is whether the format's hyperlink is an EditEngine *field*
-- broken at every cell and spilled at the ascent -- or an ordinary character property.
Writer is not EditEngine and Calc's cells are, so the honest expectation differs per row
and the point is to measure rather than to assume.
"""
import sys, zipfile
from pathlib import Path

OUT = Path(sys.argv[1] if len(sys.argv) > 1 else '/tmp/pptxfield/variants')
OUT.mkdir(parents=True, exist_ok=True)

URL = ("https://www.example.org/hr-connect/organisational-development/"
       "career-and-development-planning-framework/leadership/")

# ---------------------------------------------------------------- docx

DOCX_CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>"""

DOCX_ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOCX_DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId9" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink" Target="https://www.example.org/" TargetMode="External"/>
</Relationships>"""

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
NS = (f'{W} xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" '
      'xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" '
      'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
      'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'mc:Ignorable="mc wps"')

def wrun(text):
    return ('<w:r><w:rPr><w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/>'
            f'<w:sz w:val="32"/></w:rPr><w:t xml:space="preserve">{text}</w:t></w:r>')

# 14 cm text column: A4 (11906 twips) with 2.3 cm margins either side -> 396.85 pt
SECT = ('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1134" w:right="1758" w:bottom="1134" w:left="1758" '
        'w:header="0" w:footer="0" w:gutter="0"/></w:sectPr>')

def docx(path, body):
    doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document {NS}><w:body>{body}{SECT}</w:body></w:document>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', DOCX_CT)
        z.writestr('_rels/.rels', DOCX_ROOT_RELS)
        z.writestr('word/document.xml', doc)
        z.writestr('word/_rels/document.xml.rels', DOCX_DOC_RELS)

# w1 -- plain text control
docx(OUT / 'w1.docx', f'<w:p>{wrun(URL)}</w:p>')
# w2 -- the same URL inside a w:hyperlink
docx(OUT / 'w2.docx', f'<w:p><w:hyperlink r:id="rId9">{wrun(URL)}</w:hyperlink></w:p>')

# w3/w4 -- the same pair inside a wps text box, whose text is a Writer fly's and not
# a drawing object's; 14 cm wide so the measure matches the paragraph probes
def txbx(inner):
    return (
      '<w:p><w:r><w:rPr><w:noProof/></w:rPr>'
      '<mc:AlternateContent><mc:Choice Requires="wps"><w:drawing>'
      '<wp:inline distT="0" distB="0" distL="0" distR="0">'
      '<wp:extent cx="5040000" cy="1800000"/><wp:effectExtent l="0" t="0" r="0" b="0"/>'
      '<wp:docPr id="1" name="box"/>'
      '<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">'
      '<wps:wsp><wps:cNvSpPr txBox="1"/>'
      '<wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="5040000" cy="1800000"/></a:xfrm>'
      '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></wps:spPr>'
      f'<wps:txbx><w:txbxContent>{inner}</w:txbxContent></wps:txbx>'
      '<wps:bodyPr rot="0" lIns="0" tIns="0" rIns="0" bIns="0" anchor="t"><a:noAutofit/></wps:bodyPr>'
      '</wps:wsp></a:graphicData></a:graphic></wp:inline></w:drawing></mc:Choice></mc:AlternateContent>'
      '</w:r></w:p>')

docx(OUT / 'w3.docx', txbx(f'<w:p>{wrun(URL)}</w:p>'))
docx(OUT / 'w4.docx', txbx(f'<w:p><w:hyperlink r:id="rId9">{wrun(URL)}</w:hyperlink></w:p>'))

# ---------------------------------------------------------------- xlsx

XLSX_CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
</Types>"""

XLSX_ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>"""

XLSX_WB = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="s" sheetId="1" r:id="rId1"/></sheets></workbook>"""

XLSX_WB_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>"""

XLSX_STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="16"/><name val="Liberation Sans"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="2"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>
<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyAlignment="1"><alignment wrapText="1" vertical="top"/></xf></cellXfs>
</styleSheet>"""

SHEET_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId9" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink" Target="https://www.example.org/" TargetMode="External"/>
</Relationships>"""

def xlsx(path, link):
    hl = '<hyperlinks><hyperlink ref="A1" r:id="rId9"/></hyperlinks>' if link else ''
    sheet = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
             'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
             '<cols><col min="1" max="1" width="60" customWidth="1"/></cols>'
             '<sheetData><row r="1" customHeight="1" ht="120">'
             f'<c r="A1" s="1" t="inlineStr"><is><t>{URL}</t></is></c></row></sheetData>'
             f'{hl}</worksheet>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', XLSX_CT)
        z.writestr('_rels/.rels', XLSX_ROOT_RELS)
        z.writestr('xl/workbook.xml', XLSX_WB)
        z.writestr('xl/_rels/workbook.xml.rels', XLSX_WB_RELS)
        z.writestr('xl/styles.xml', XLSX_STYLES)
        z.writestr('xl/worksheets/sheet1.xml', sheet)
        if link:
            z.writestr('xl/worksheets/_rels/sheet1.xml.rels', SHEET_RELS)

xlsx(OUT / 'x1.xlsx', False)
xlsx(OUT / 'x2.xlsx', True)
print('w1 w2 w3 w4 x1 x2 written to', OUT)
