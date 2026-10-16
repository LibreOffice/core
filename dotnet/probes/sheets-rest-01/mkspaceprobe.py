#!/usr/bin/env python3
"""Build an XLSX probe that varies only the *trailing whitespace* of a wrapped cell.

The question: how many lines does LibreOffice's EditEngine put a run of spaces on when
it is wider than the column? A row whose file states no `customHeight` is recomputed on
import, so the answer is readable straight out of `soffice --convert-to fods` as
`style:row-height`.

Column A reproduces `FAA-2019-0995-0002_attachment_2.xlsx`'s column S exactly — width
13.7109375 digits, Arial 8, `wrapText`. Column B carries a one-character marker in a
narrow non-wrapping column so the same row pitch can be read out of a *rendered* PDF,
which is how our own answer is measured.
"""
import os
import sys
import zipfile

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

WBRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>'''

WB = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="S1" sheetId="1" r:id="rId1"/></sheets>
</workbook>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="2"><font><sz val="10"/><name val="Arial"/></font><font><sz val="8"/><name val="Arial"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="3">
<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>
<xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1" applyAlignment="1"><alignment wrapText="1"/></xf>
<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyAlignment="1"><alignment vertical="top"/></xf>
</cellXfs>
</styleSheet>'''


def build(path, strings):
    sst = ''.join(
        '<si><t xml:space="preserve">%s</t></si>' % s.replace('&', '&amp;').replace('<', '&lt;')
        for s in strings)
    sstxml = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
              'count="%d" uniqueCount="%d">%s</sst>' % (len(strings), len(strings), sst))

    rows = []
    for i, _ in enumerate(strings):
        r = i + 1
        rows.append(
            '<row r="%d" spans="1:2">'
            '<c r="A%d" s="1" t="s"><v>%d</v></c>'
            '<c r="B%d" s="2" t="str"><v>M</v></c>'
            '</row>' % (r, r, i, r))

    sheet = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
             '<sheetFormatPr defaultColWidth="11.42578125" defaultRowHeight="12.75"/>'
             '<cols>'
             '<col min="1" max="1" width="13.7109375" style="1" customWidth="1"/>'
             '<col min="2" max="2" width="6" style="2" customWidth="1"/>'
             '</cols>'
             '<sheetData>%s</sheetData>'
             '<pageSetup paperSize="9" orientation="portrait"/>'
             '</worksheet>' % ''.join(rows))

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('xl/workbook.xml', WB)
        z.writestr('xl/_rels/workbook.xml.rels', WBRELS)
        z.writestr('xl/styles.xml', STYLES)
        z.writestr('xl/sharedStrings.xml', sstxml)
        z.writestr('xl/worksheets/sheet1.xml', sheet)


CASES = [
    ('empty', ''),
    ('x', 'x'),
    ('sp10', ' ' * 10),
    ('sp20', ' ' * 20),
    ('sp30', ' ' * 30),
    ('sp40', ' ' * 40),
    ('sp60', ' ' * 60),
    ('sp80', ' ' * 80),
    ('sp100', ' ' * 100),
    ('sp120', ' ' * 120),
    ('sp160', ' ' * 160),
    ('sp200', ' ' * 200),
    ('word+sp83', 'inspected in situ' + ' ' * 83),
    ('abc+sp97', 'abc' + ' ' * 97),
    ('M40', 'M' * 40),
    ('M40+sp60', 'M' * 40 + ' ' * 60),
    ('lead100+x', ' ' * 100 + 'x'),
    ('words', 'alpha beta gamma delta epsilon zeta eta theta'),
]

if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    build(os.path.join(out, 'spaceprobe.xlsx'), [s for _, s in CASES])
    for i, (name, s) in enumerate(CASES):
        print('%d\t%s\tlen=%d' % (i + 1, name, len(s)))
