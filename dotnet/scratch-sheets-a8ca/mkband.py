#!/usr/bin/env python3
"""Build sheet-footer-band.xlsx.

Shape, not a copy of any corpus workbook. Letter portrait, 0.75 in top and bottom
margins with 0.3 in header/footer margins, so the footer's stated band is 0.45 in
(32.4 pt) and the body the margins imply is 792 - 54 - 21.6 - 32.4 = 684.0 pt.

The footer is one line at 20 pt. Calc's import measures that line as its bare point
size, 20 pt, and stores bodyDistance = 32.4 - 20 = 12.4 pt; at print time it measures
the line properly (22.2 pt for Liberation Sans at 20 pt) and the band becomes 34.6 pt,
leaving a body of 681.8 pt.

The rows sum to 13658 twips = 682.9 pt, which is inside (681.8, 684.0]: it fits the
band the margins imply and does not fit the band that prints. One page against two.
"""
import zipfile, sys

OUT = sys.argv[1]

ROWS = [(r, 15.0) for r in range(1, 46)] + [(46, 7.9)]   # 45*300 + 158 = 13658 twips

def rowxml(r, ht, text):
    return ('<row r="%d" ht="%s" customHeight="1"><c r="A%d" t="inlineStr">'
            '<is><t>%s</t></is></c></row>' % (r, ht, r, text))

sheet = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
    '<dimension ref="A1:A46"/>'
    '<sheetFormatPr defaultRowHeight="15"/>'
    '<cols><col min="1" max="1" width="40" customWidth="1"/></cols>'
    '<sheetData>'
    + ''.join(rowxml(r, ht, 'Row %d of the band fixture' % r) for r, ht in ROWS) +
    '</sheetData>'
    '<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75" header="0.3" footer="0.3"/>'
    '<pageSetup paperSize="1" orientation="portrait"/>'
    '<headerFooter><oddFooter>&amp;C&amp;20Footer band</oddFooter></headerFooter>'
    '</worksheet>')

workbook = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"'
    ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
    '<sheets><sheet name="Band" sheetId="1" r:id="rId1"/></sheets></workbook>')

styles = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
    '<fonts count="1"><font><sz val="10"/><name val="Arial"/></font></fonts>'
    '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
    '<borders count="1"><border/></borders>'
    '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
    '<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>'
    '</styleSheet>')

content_types = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
    '<Default Extension="xml" ContentType="application/xml"/>'
    '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
    '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
    '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
    '</Types>')

root_rels = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>'
    '</Relationships>')

wb_rels = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>'
    '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
    '</Relationships>')

with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', content_types)
    z.writestr('_rels/.rels', root_rels)
    z.writestr('xl/workbook.xml', workbook)
    z.writestr('xl/_rels/workbook.xml.rels', wb_rels)
    z.writestr('xl/worksheets/sheet1.xml', sheet)
    z.writestr('xl/styles.xml', styles)

twips = sum(round(ht * 20) for _, ht in ROWS)
print('%s: %d rows, %d twips = %.1f pt' % (OUT, len(ROWS), twips, twips / 20))
