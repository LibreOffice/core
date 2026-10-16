#!/usr/bin/env python3
"""Build `sheet-hyperlink-underline.xlsx`, the fixture O62's tests read.

Five cells in one column, each one arm of the rule under test:

  A1  linked, font states no underline    -> ONE rule (the seat: the field adds it)
  A2  unlinked, same font                 -> no rule at all (the control)
  A3  linked, font states a DOUBLE line   -> ONE rule (SetUnderline replaces, not maxes)
  A4  unlinked, same double font          -> TWO rules (the control for A3)
  A5  linked, but the cell holds a NUMBER -> no rule; `insertHyperlink` converts only a
                                             string cell, so a numeric one keeps a plain
                                             ATTR_HYPERLINK and is drawn as an ordinary cell

Arial 12, no stated colour: black is what every cell would be drawn in without the field
rule, so the link colour and the link's underline are each visible on their own.  Every
string is distinct so a run can be attributed from the drawn text alone.
"""
import os, sys, zipfile

OUT = sys.argv[1] if len(sys.argv) > 1 else 'sheet-hyperlink-underline.xlsx'
H = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
M = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
P = 'http://schemas.openxmlformats.org/package/2006/relationships'
LINK = R + '/hyperlink'
STRINGS = ['linkplain', 'bareplain', 'linkdouble', 'baredouble']

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
  '<Relationships xmlns="%s">'
  '<Relationship Id="rId1" Type="%s/officeDocument" Target="xl/workbook.xml"/>'
  '</Relationships>' % (P, R)),
 'xl/_rels/workbook.xml.rels': H + (
  '<Relationships xmlns="%s">'
  '<Relationship Id="rId1" Type="%s/worksheet" Target="worksheets/sheet1.xml"/>'
  '<Relationship Id="rId2" Type="%s/styles" Target="styles.xml"/>'
  '<Relationship Id="rId3" Type="%s/sharedStrings" Target="sharedStrings.xml"/>'
  '</Relationships>' % (P, R, R, R)),
 'xl/workbook.xml': H + (
  '<workbook xmlns="%s" xmlns:r="%s">'
  '<sheets><sheet name="Probe" sheetId="1" r:id="rId1"/></sheets></workbook>' % (M, R)),
 'xl/sharedStrings.xml': H + (
  '<sst xmlns="%s" count="%d" uniqueCount="%d">%s</sst>'
  % (M, len(STRINGS), len(STRINGS),
     ''.join('<si><t>%s</t></si>' % s for s in STRINGS))),
 'xl/styles.xml': H + (
  '<styleSheet xmlns="%s">'
  '<fonts count="2">'
  '<font><sz val="12"/><name val="Arial"/><family val="2"/></font>'
  '<font><u val="double"/><sz val="12"/><name val="Arial"/><family val="2"/></font>'
  '</fonts>'
  '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
  '<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>'
  '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
  '<cellXfs count="2">'
  '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/>'
  '<xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1"/>'
  '</cellXfs></styleSheet>' % M),
 'xl/worksheets/_rels/sheet1.xml.rels': H + (
  '<Relationships xmlns="%s">'
  '<Relationship Id="rId1" Type="%s" Target="https://www.example.org/a" TargetMode="External"/>'
  '<Relationship Id="rId2" Type="%s" Target="https://www.example.org/b" TargetMode="External"/>'
  '<Relationship Id="rId3" Type="%s" Target="https://www.example.org/c" TargetMode="External"/>'
  '</Relationships>' % (P, LINK, LINK, LINK)),
 'xl/worksheets/sheet1.xml': H + (
  '<worksheet xmlns="%s" xmlns:r="%s">'
  '<dimension ref="A1:A5"/>'
  '<cols><col min="1" max="1" width="24" customWidth="1"/></cols>'
  '<sheetData>'
  '<row r="1"><c r="A1" s="0" t="s"><v>0</v></c></row>'
  '<row r="2"><c r="A2" s="0" t="s"><v>1</v></c></row>'
  '<row r="3"><c r="A3" s="1" t="s"><v>2</v></c></row>'
  '<row r="4"><c r="A4" s="1" t="s"><v>3</v></c></row>'
  '<row r="5"><c r="A5" s="0"><v>1205</v></c></row>'
  '</sheetData>'
  '<hyperlinks>'
  '<hyperlink ref="A1" r:id="rId1"/>'
  '<hyperlink ref="A3" r:id="rId2"/>'
  '<hyperlink ref="A5" r:id="rId3"/>'
  '</hyperlinks>'
  '<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75" header="0.3" footer="0.3"/>'
  '</worksheet>' % (M, R)),
}

os.makedirs(os.path.dirname(os.path.abspath(OUT)), exist_ok=True)
with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED) as z:
    for name, body in parts.items():
        info = zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0))
        info.compress_type = zipfile.ZIP_DEFLATED
        z.writestr(info, body)
print('wrote', OUT)
