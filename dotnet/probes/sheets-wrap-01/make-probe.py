#!/usr/bin/env python3
"""Builds sheet-wrap-fields.xlsx: the same six strings, once plain and once as a
sheet-level hyperlink, in one wrap-enabled column of fixed width.

The point is to separate two candidate explanations for a hyperlink cell drawing
one clipped line where LibreOffice draws two: (a) a missing character-break
fallback, which would fail for every space-free token; (b) the field being atomic,
which would fail only for the hyperlink arm.
"""
import sys, zipfile

AUTO = "--auto-height" in sys.argv   # omit ht/customHeight so Calc computes optimal heights

URL   = "https://www.bsp.gov.ph/Regulations/Published%20Issuances/Images/M-2024-039.pdf"
PLAIN = "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIIIJJJJKKKKLLLLMMMMNNNNOOOOPPPP"
WORDS = "alpha bravo charlie delta echo foxtrot golf hotel india juliet kilo lima"

# (text, hyperlinked)
CELLS = [
    (URL,   False),
    (URL,   True),
    (PLAIN, False),
    (PLAIN, True),
    (WORDS, False),
    (WORDS, True),
]

NS = 'xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"'
R  = 'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'

strings = [t for t, _ in CELLS]

rows, links, rels = [], [], []
for i, (text, linked) in enumerate(CELLS, start=1):
    height = '' if AUTO else ' ht="72" customHeight="1"'
    rows.append(f'<row r="{i}"{height}>'
                f'<c r="A{i}" s="1" t="s"><v>{i-1}</v></c></row>')
    if linked:
        rid = f"rId{i}"
        links.append(f'<hyperlink ref="A{i}" r:id="{rid}"/>')
        rels.append(f'<Relationship Id="{rid}" '
                    'Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                    'relationships/hyperlink" '
                    f'Target="{text}" TargetMode="External"/>')

sheet = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
         f'<worksheet {NS} {R}><dimension ref="A1:A{len(CELLS)}"/>'
         '<sheetFormatPr defaultRowHeight="11.4"/>'
         '<cols><col min="1" max="1" width="30" customWidth="1"/></cols>'
         f'<sheetData>{"".join(rows)}</sheetData>'
         f'<hyperlinks>{"".join(links)}</hyperlinks>'
         '<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75" '
         'header="0.3" footer="0.3"/></worksheet>')

sst = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       f'<sst {NS} count="{len(strings)}" uniqueCount="{len(strings)}">'
       + "".join(f'<si><t xml:space="preserve">{s.replace("&","&amp;")}</t></si>'
                 for s in strings)
       + '</sst>')

styles = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><styleSheet {NS}>'
          '<fonts count="1"><font><sz val="10"/><name val="Calibri"/></font></fonts>'
          '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
          '<borders count="1"><border/></borders>'
          '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
          '<cellXfs count="2">'
          '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>'
          '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyAlignment="1">'
          '<alignment horizontal="left" vertical="top" wrapText="1"/></xf>'
          '</cellXfs>'
          '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
          '</styleSheet>')

parts = {
 "[Content_Types].xml":
   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
   '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
   '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
   '<Default Extension="xml" ContentType="application/xml"/>'
   '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
   '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
   '<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>'
   '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
   '</Types>',
 "_rels/.rels":
   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>'
   '</Relationships>',
 "xl/workbook.xml":
   f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><workbook {NS} {R}>'
   '<sheets><sheet name="Probe" sheetId="1" r:id="rId1"/></sheets></workbook>',
 "xl/_rels/workbook.xml.rels":
   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>'
   '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>'
   '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
   '</Relationships>',
 "xl/worksheets/sheet1.xml": sheet,
 "xl/worksheets/_rels/sheet1.xml.rels":
   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
   + "".join(rels) + '</Relationships>',
 "xl/sharedStrings.xml": sst,
 "xl/styles.xml": styles,
}

out = sys.argv[1]
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
    for name, data in parts.items():
        z.writestr(name, data)
print("wrote", out)
