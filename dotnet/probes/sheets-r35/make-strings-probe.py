#!/usr/bin/env python3
"""A workbook of prefixes of one string in one wrapping column.

Each row holds a prefix of the target string, so LibreOffice's own `style:row-height` for that
row states how many lines *it* breaks the prefix into. Comparing against our resolved heights
localises the first character at which the two break differently.

Usage: make-prefix-probe.py <out.xlsx> <width-digits> <string-file> [step]
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
<sheets><sheet name="Prefixes" sheetId="1" r:id="rId1"/></sheets>
</workbook>'''
STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="2"><font><sz val="11"/><name val="Calibri"/><family val="2"/></font><font><sz val="10"/><name val="Calibri"/><family val="2"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="1">
<xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1" applyAlignment="1"><alignment wrapText="1" vertical="bottom"/></xf>
</cellXfs>
</styleSheet>'''

out, width, path = sys.argv[1], sys.argv[2], sys.argv[3]
step = int(sys.argv[4]) if len(sys.argv) > 4 else 1

prefixes = [l for l in open(path, encoding='utf-8').read().split('\n') if l != '']

def esc(s):
    return (s.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;'))

sst = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
       '<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="%d" uniqueCount="%d">%s</sst>'
       % (len(prefixes), len(prefixes),
          ''.join('<si><t xml:space="preserve">%s</t></si>' % esc(s) for s in prefixes)))
rows = ''.join('<row r="%d" ht="15" spans="1:1"><c r="A%d" s="0" t="s"><v>%d</v></c></row>'
               % (i + 1, i + 1, i) for i in range(len(prefixes)))
SHEET = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<cols><col min="1" max="1" width="%s" customWidth="1"/></cols>
<sheetData>%s</sheetData>
</worksheet>''' % (width, rows)

with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('xl/workbook.xml', WB)
    z.writestr('xl/_rels/workbook.xml.rels', WB_RELS)
    z.writestr('xl/styles.xml', STYLES)
    z.writestr('xl/sharedStrings.xml', sst)
    z.writestr('xl/worksheets/sheet1.xml', SHEET)
print(out, len(prefixes), 'rows')
