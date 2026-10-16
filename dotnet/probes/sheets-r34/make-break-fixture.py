#!/usr/bin/env python3
"""Builds `sheet-row-height-break.xlsx`, the fixture the round's two rules are pinned against.

Written as SpreadsheetML directly rather than authored as flat ODF and converted, because the
whole point of the fixture is that **the SpreadsheetML importer folds a string holding U+000A
into one paragraph while the ODF importer makes it a multi-paragraph edit cell**
(`SheetDataBuffer::setStringCell`, `sc/source/filter/oox/sheetdatabuffer.cxx:120-135`, reached by
every string U+000A because `RichString::extractPlainString` refuses one at `richstring.cxx:375`;
against `ScXMLImport`'s several `text:p`). A fixture converted through ODF would measure the
other importer and pass whatever the code did.

The face is stated explicitly — Liberation Sans at 10 pt — because a fixture naming Arial
resolves to the unwired default and would pass whatever the code did, for a second reason.

Four rows, one column, all four with a `ht` hint and **no** `customHeight`, so Calc discards
every stated height and measures the content again:

    row 1  Alpha\\nBravo\\nCharlie   no wrap    one line   — the break is the format's to grant
    row 2  Alpha\\nBravo\\nCharlie   wrap       three lines
    row 3  Alpha\\nBravo\\n          wrap       three lines — the trailing break is a paragraph
    row 4  Alpha\\nBravo            wrap       two lines
    row 5  Alpha                  no wrap    one line   — the control row 1 is compared against

Row 5 exists so that row 1 can be asserted against a row with no break in it rather than
against a constant. The two are the same height for the same reason and the assertion then
does not also depend on the sheet's optimal-minimum floor, which is a separate quantity and a
separate open question: LibreOffice writes 252.9 twips for both of these rows and we write
256, because we default the floor to the standard row height where Calc defaults it to none.
"""
import sys, zipfile

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

WB_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>'''

WB = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="Breaks" sheetId="1" r:id="rId1"/></sheets>
</workbook>'''

# Two cell formats over one font: xf 0 does not wrap, xf 1 does. The font is stated on the
# workbook's only font entry so that nothing falls through to an application default.
STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="10"/><name val="Liberation Sans"/><family val="2"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="2">
<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyAlignment="1"><alignment wrapText="0" vertical="bottom"/></xf>
<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyAlignment="1"><alignment wrapText="1" vertical="bottom"/></xf>
</cellXfs>
</styleSheet>'''

STRINGS = ['Alpha\nBravo\nCharlie', 'Alpha\nBravo\nCharlie', 'Alpha\nBravo\n', 'Alpha\nBravo',
           'Alpha']
XF = [0, 1, 1, 1, 0]

sst = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="%d" uniqueCount="%d">%s</sst>' % (
    len(STRINGS), len(STRINGS),
    ''.join('<si><t xml:space="preserve">%s</t></si>' % s.replace('\n', '&#10;') for s in STRINGS))

rows = ''.join(
    '<row r="%d" ht="15" spans="1:1"><c r="A%d" s="%d" t="s"><v>%d</v></c></row>' % (i + 1, i + 1, XF[i], i)
    for i in range(len(STRINGS)))

# One narrow column, so that row 2's three paragraphs each stay on one line and the wrap under
# test is the hard break rather than the width.
SHEET = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<cols><col min="1" max="1" width="20" customWidth="1"/></cols>
<sheetData>%s</sheetData>
</worksheet>''' % rows

out = sys.argv[1]
with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('xl/workbook.xml', WB)
    z.writestr('xl/_rels/workbook.xml.rels', WB_RELS)
    z.writestr('xl/styles.xml', STYLES)
    z.writestr('xl/sharedStrings.xml', sst)
    z.writestr('xl/worksheets/sheet1.xml', SHEET)
print(out)
