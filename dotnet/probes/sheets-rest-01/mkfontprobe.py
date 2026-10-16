#!/usr/bin/env python3
"""Build an XLSX probe whose `<font>` entries omit `name`, `sz`, or both.

`ThemeBuffer::ThemeBuffer` (`sc/source/filter/oox/themebuffer.cxx:31-33`) seeds every
`<font>` the OOXML filter builds with a hard-coded `Cambria`, 11.0 pt — so a `<font>` that
states neither is Cambria 11 rather than the workbook's own `fonts[0]`. This measures
whether the installed binary really does that, and what each omission is worth on its own.

Read the answer out of `soffice --convert-to fods`: each cell carries a
`style:font-name` / `fo:font-size` in its automatic style.
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

# fonts[0] is the workbook default and is deliberately NOT Cambria, so the two candidate
# answers — "the theme's Cambria 11" and "the workbook's own fonts[0]" — are distinguishable.
FONTS = [
    ('base', '<font><sz val="10"/><name val="Arial"/></font>'),
    ('bare', '<font/>'),
    ('bold-only', '<font><b/></font>'),
    ('size-only', '<font><sz val="20"/></font>'),
    ('name-only', '<font><name val="Arial"/></font>'),
    ('colour-only', '<font><color rgb="FF999999"/></font>'),
    ('underline-only', '<font><u/><color rgb="FF0000FF"/></font>'),
]


def build(path):
    fonts = ''.join(f for _, f in FONTS)
    styles = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
        '<fonts count="%d">%s</fonts>'
        '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
        '<borders count="1"><border/></borders>'
        '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
        '<cellXfs count="%d">%s</cellXfs>'
        '</styleSheet>' % (
            len(FONTS), fonts, len(FONTS),
            ''.join('<xf numFmtId="0" fontId="%d" fillId="0" borderId="0" xfId="0" applyFont="1"/>'
                    % i for i in range(len(FONTS)))))

    names = [n for n, _ in FONTS]
    sst = ''.join('<si><t>%s</t></si>' % n for n in names)
    sstxml = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
              'count="%d" uniqueCount="%d">%s</sst>' % (len(names), len(names), sst))

    rows = ''.join('<row r="%d"><c r="A%d" s="%d" t="s"><v>%d</v></c></row>' % (i + 1, i + 1, i, i)
                   for i in range(len(FONTS)))
    sheet = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
             '<sheetData>%s</sheetData>'
             '<pageSetup paperSize="9" orientation="portrait"/></worksheet>' % rows)

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('xl/workbook.xml', WB)
        z.writestr('xl/_rels/workbook.xml.rels', WBRELS)
        z.writestr('xl/styles.xml', styles)
        z.writestr('xl/sharedStrings.xml', sstxml)
        z.writestr('xl/worksheets/sheet1.xml', sheet)


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    build(os.path.join(out, 'fontprobe.xlsx'))
    for i, (name, xml) in enumerate(FONTS):
        print('%d\t%s\t%s' % (i + 1, name, xml))
