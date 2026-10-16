#!/usr/bin/env python3
"""make-pitch-probe.py <out.xlsx> <font> <size>

One hyperlinked, wrap-enabled cell in a column too narrow for its content, in a row tall
enough to hold every line it wraps into.

The cell holds a run of 'X' — one token with no break opportunity, so every line is a chop
and every line has the identical glyph repertoire. The vertical gap between two lines' glyph
bounding boxes is then exactly the line pitch, with no ascender or descender bias to correct
for, which is what makes a 0.02 pt reading meaningful.
"""
import sys, zipfile

out, family, size = sys.argv[1], sys.argv[2], float(sys.argv[3])
TEXT = "X" * 40

NS = 'xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"'
R = 'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'

sheet = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><worksheet {NS} {R}>'
         '<dimension ref="A1:A1"/><sheetFormatPr defaultRowHeight="11.4"/>'
         '<cols><col min="1" max="1" width="14" customWidth="1"/></cols>'
         '<sheetData><row r="1" ht="420" customHeight="1">'
         '<c r="A1" s="1" t="s"><v>0</v></c></row></sheetData>'
         '<hyperlinks><hyperlink ref="A1" r:id="rId1"/></hyperlinks>'
         '<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75" header="0.3" footer="0.3"/>'
         '</worksheet>')

styles = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><styleSheet {NS}>'
          '<fonts count="2"><font><sz val="10"/><name val="Calibri"/></font>'
          f'<font><sz val="{size:g}"/><name val="{family}"/></font></fonts>'
          '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
          '<borders count="1"><border/></borders>'
          '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
          '<cellXfs count="2"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>'
          '<xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1" '
          'applyAlignment="1"><alignment horizontal="left" vertical="top" wrapText="1"/></xf>'
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
   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink" '
   'Target="https://example.org/x" TargetMode="External"/></Relationships>',
 "xl/sharedStrings.xml":
   f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><sst {NS} count="1" uniqueCount="1">'
   f'<si><t>{TEXT}</t></si></sst>',
 "xl/styles.xml": styles,
}

with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
    for name, data in parts.items():
        z.writestr(name, data)
