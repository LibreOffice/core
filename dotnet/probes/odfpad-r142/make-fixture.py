#!/usr/bin/env python3
"""Build `features/sheet-odf-numfmt-padding.ods`, the O99 fixture.

Written as an `.xlsx` and converted by **26.2.4.2**, because what is under test
is how LibreOffice's ODF exporter writes the two padding directives -- and it
writes neither of them as a directive. `_x` ("leave the width of x blank") has
no ODF spelling at all: the exporter emits the SPACES into a `number:text` and
records what they stood for in `loext:blank-width-char`, as a sequence of
`<char>[<position>]` groups separated by `_`. `*x` ("repeat x to fill the
column") becomes a `number:fill-character` element holding the character.

Four formats, and what each separates:

  A  `_(* #,##0_);_(* (#,##0);_(* "-"_);_(@_)`
        the ASCII accounting format. Every blank is one character wide, the
        groups are single characters with no position, and one of them states
        a position (`)1`) because its text is two characters long.

  B  `_-* #,##0.00\\ _€_-;-* #,##0.00\\ _€_-;_-* "-"?? _€_-;_-@_-`
        the euro accounting format, which is the reach that matters: `€` is
        above ASCII, so `InsertBlanks` writes **two** spaces for it and not one,
        and its trailing text carries a MULTI-GROUP spec with positions --
        `€1_-3` over four spaces. A reader that removes one character per group,
        or that ignores the positions, lands the directive in the wrong place.

  C  `* #,##0`
        a fill character with no blanks beside it, so the two are separable.

  D  `0.00`
        neither, and the control that must not move.
"""
import zipfile, pathlib, subprocess, sys, shutil, tempfile

R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
SOFFICE = '/opt/libreoffice26.2/program/soffice'

FORMATS = [
    '_(* #,##0_);_(* (#,##0);_(* &quot;-&quot;_);_(@_)',
    '_-* #,##0.00\\ _€_-;-* #,##0.00\\ _€_-;_-* &quot;-&quot;?? _€_-;_-@_-',
    '* #,##0',
    '0.00',
]
VALUES = [1234.5, -1234.5, 0.0, None]
COLUMNS = 'ABCD'


def parts():
    numfmts = ''.join(
        f'<numFmt numFmtId="{164 + i}" formatCode="{c}"/>' for i, c in enumerate(FORMATS))
    xfs = ''.join(
        f'<xf numFmtId="{164 + i}" applyNumberFormat="1" xfId="0"/>' for i in range(len(FORMATS)))

    rows = []
    for r, value in enumerate(VALUES, start=1):
        cells = []
        for c, column in enumerate(COLUMNS):
            if value is None:
                cells.append(f'<c r="{column}{r}" s="{c}" t="inlineStr"><is><t>text</t></is></c>')
            else:
                cells.append(f'<c r="{column}{r}" s="{c}"><v>{value}</v></c>')
        rows.append(f'<row r="{r}">' + ''.join(cells) + '</row>')

    return {
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
'xl/styles.xml': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<numFmts count="{len(FORMATS)}">{numfmts}</numFmts>
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf/></cellStyleXfs>
<cellXfs count="{len(FORMATS)}">{xfs}</cellXfs>
</styleSheet>''',
'xl/worksheets/sheet1.xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<cols><col min="1" max="4" width="18" customWidth="1"/></cols>
<sheetData>
''' + '\n'.join(rows) + '''
</sheetData>
</worksheet>''',
    }


def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1
                       else 'dotnet/tests/corpus/features/sheet-odf-numfmt-padding.ods')
    out.parent.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as work:
        work = pathlib.Path(work)
        book = work / 'sheet-odf-numfmt-padding.xlsx'
        with zipfile.ZipFile(book, 'w', zipfile.ZIP_DEFLATED) as z:
            for name, text in parts().items():
                z.writestr(name, text)

        subprocess.run(
            [SOFFICE, '--headless', f'-env:UserInstallation=file://{work}/profile',
             '--convert-to', 'ods', '--outdir', str(work), str(book)],
            check=True, capture_output=True, timeout=300)

        made = work / 'sheet-odf-numfmt-padding.ods'
        if not made.exists():
            raise SystemExit('conversion produced nothing -- assert the instrument ran')
        shutil.copy(made, out)

    print(f'wrote {out} ({out.stat().st_size} bytes)')


if __name__ == '__main__':
    main()
