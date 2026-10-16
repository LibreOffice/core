#!/usr/bin/env python3
"""Build `features/sheet-numfmt-colour.xlsx`, the O92 fixture.

One sheet, one custom `numFmt` stating a colour on two of its three sections:

    0.0;[Red]-0.0;[Blue]0.0

and three cells choosing between them by value -- 1.5 positive (no colour, the
control), -2.5 negative ([Red]) and 0 ([Blue]). A fourth cell carries the same
value as the negative one under a colourless format, so a run that coloured by
VALUE rather than by the selected SECTION would fail on it.

`[Blue]` is deliberately the third section: LibreOffice maps BLUE to
COL_LIGHTBLUE (#0000FF) and not to COL_BLUE (#000080), and a fixture using only
red cannot tell the two tables apart.
"""
import struct, zlib, zipfile, pathlib, sys

R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
CELLS = [('A1', 0, 1.5), ('A2', 0, -2.5), ('A3', 0, 0), ('A4', 1, -2.5)]

PARTS = {
'[Content_Types].xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
</Types>''',
'_rels/.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/officeDocument" Target="xl/workbook.xml"/>
</Relationships>''',
'xl/workbook.xml': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="{R}">
<sheets><sheet name="Probe" sheetId="1" r:id="rId1"/></sheets>
</workbook>''',
'xl/_rels/workbook.xml.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="{R}/styles" Target="styles.xml"/>
</Relationships>''',
# numFmtId 164 carries the colours; xf 0 uses it, xf 1 is the colourless control.
'xl/styles.xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<numFmts count="1"><numFmt numFmtId="164" formatCode="0.0;[Red]-0.0;[Blue]0.0"/></numFmts>
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf/></cellStyleXfs>
<cellXfs count="2">
<xf numFmtId="164" applyNumberFormat="1" xfId="0"/>
<xf numFmtId="2" applyNumberFormat="1" xfId="0"/>
</cellXfs>
</styleSheet>''',
'xl/worksheets/sheet1.xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<sheetData>
__ROWS__
</sheetData>
</worksheet>''',
}

def main():
    rows = '\n'.join(
        f'<row r="{i+1}"><c r="{ref}" s="{s}"><v>{v}</v></c></row>'
        for i, (ref, s, v) in enumerate(CELLS))
    parts = dict(PARTS)
    parts['xl/worksheets/sheet1.xml'] = parts['xl/worksheets/sheet1.xml'].replace('__ROWS__', rows)
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1
                       else 'dotnet/tests/corpus/features/sheet-numfmt-colour.xlsx')
    out.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in parts.items():
            z.writestr(name, text)
    print(f'wrote {out} ({out.stat().st_size} bytes)')

if __name__ == '__main__':
    main()
