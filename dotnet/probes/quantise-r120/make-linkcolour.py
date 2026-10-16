#!/usr/bin/env python3
"""Build `link-colour.xlsx`: which hyperlink cells keep the file's own colour.

    make-linkcolour.py <out.xlsx>

O65 asks why one workbook's hyperlink cells are drawn in the file's blue where every other
workbook's are drawn in the application's navy.  Six cells, each an arm:

  A1  plain string, linked, no stated colour        -- the base case
  A2  plain string, linked, stated #FF0000
  A3  rich string, linked, one run stated #00B050
  A4  rich string, linked, two runs stating nothing (`splitA`+`splitB`)
  A5  plain string, NOT linked, stated #FF0000      -- control
  A6  rich string, NOT linked, one run #00B050      -- control

The same file is then converted to `.xls` by the reference itself, so both spreadsheet
importers are measured on identical content -- which is the whole question, because the two
build the field differently: `WorksheetGlobals::insertHyperlink`
(`sc/source/filter/oox/worksheethelper.cxx`) throws the cell's rich text away and inserts a
bare field, while `lclInsertUrl` (`sc/source/filter/excel/xicontent.cxx`:155-215) keeps the
edit object, or applies the cell pattern's own item set to it.

No run's text is a substring of another cell's label, so a test can attribute a run to a cell
from the drawn text alone -- which it has to, because this tree draws a rich cell as one run
per portion where the reference draws the whole hyperlink cell as one field.
"""
import sys, zipfile

OUT = sys.argv[1] if len(sys.argv) > 1 else 'link-colour.xlsx'
H = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
M = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
P = 'http://schemas.openxmlformats.org/package/2006/relationships'

FONT = '<sz val="12"/><name val="Arial"/><family val="2"/>'
SI = [
    '<si><t>plainauto</t></si>',
    '<si><t>plainred</t></si>',
    '<si><r><rPr><color rgb="FF00B050"/>%s</rPr><t>richgreen</t></r></si>' % FONT,
    '<si><r><rPr>%s</rPr><t>splitA</t></r><r><rPr>%s</rPr><t>splitB</t></r></si>' % (FONT, FONT),
    '<si><t>barered</t></si>',
    '<si><r><rPr><color rgb="FF00B050"/>%s</rPr><t>bargreen</t></r></si>' % FONT,
]
# style 0 no colour, style 1 red
ROWS = [(1, 0), (2, 1), (3, 0), (4, 0), (5, 1), (6, 0)]
LINKED = [1, 2, 3, 4]

rows = ''.join(
    '<row r="%d"><c r="A%d" t="s" s="%d"><v>%d</v></c></row>' % (r, r, s, r - 1)
    for r, s in ROWS)
links = ''.join(
    '<hyperlink ref="A%d" r:id="rId%d"/>' % (r, r) for r in LINKED)
linkrels = ''.join(
    '<Relationship Id="rId%d" Type="%s/hyperlink" Target="https://example.invalid/%d" '
    'TargetMode="External"/>' % (r, R, r) for r in LINKED)

parts = {
 '[Content_Types].xml': H + (
  '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
  '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
  '<Default Extension="xml" ContentType="application/xml"/>'
  '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
  '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
  '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
  '<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>'
  '</Types>'),
 '_rels/.rels': H + (
  '<Relationships xmlns="%s"><Relationship Id="rId1" Type="%s/officeDocument" '
  'Target="xl/workbook.xml"/></Relationships>' % (P, R)),
 'xl/_rels/workbook.xml.rels': H + (
  '<Relationships xmlns="%s">'
  '<Relationship Id="rId1" Type="%s/worksheet" Target="worksheets/sheet1.xml"/>'
  '<Relationship Id="rId2" Type="%s/styles" Target="styles.xml"/>'
  '<Relationship Id="rId3" Type="%s/sharedStrings" Target="sharedStrings.xml"/>'
  '</Relationships>' % (P, R, R, R)),
 'xl/worksheets/_rels/sheet1.xml.rels': H + (
  '<Relationships xmlns="%s">%s</Relationships>' % (P, linkrels)),
 'xl/workbook.xml': H + (
  '<workbook xmlns="%s" xmlns:r="%s"><sheets>'
  '<sheet name="Probe" sheetId="1" r:id="rId1"/></sheets></workbook>' % (M, R)),
 'xl/sharedStrings.xml': H + (
  '<sst xmlns="%s" count="%d" uniqueCount="%d">%s</sst>' % (M, len(SI), len(SI), ''.join(SI))),
 'xl/styles.xml': H + (
  '<styleSheet xmlns="%s">'
  '<fonts count="2"><font>%s</font><font><color rgb="FFFF0000"/>%s</font></fonts>'
  '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
  '<borders count="1"><border/></borders>'
  '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
  '<cellXfs count="2">'
  '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/>'
  '<xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1"/>'
  '</cellXfs></styleSheet>' % (M, FONT, FONT)),
 'xl/worksheets/sheet1.xml': H + (
  '<worksheet xmlns="%s" xmlns:r="%s"><sheetData>%s</sheetData>'
  '<hyperlinks>%s</hyperlinks></worksheet>' % (M, R, rows, links)),
}

with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED) as z:
    for name, data in parts.items():
        z.writestr(name, data)
print(OUT)
