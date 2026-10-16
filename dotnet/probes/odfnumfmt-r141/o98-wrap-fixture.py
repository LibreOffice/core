#!/usr/bin/env python3
"""Build the O96 fixture, in two steps.

Step one writes an `.xlsx` stating six number formats and the cells that choose
between their sections. Step two is `soffice --convert-to ods` through
**26.2.4.2**, and the committed file is that `.ods` -- because the thing under
test is how LibreOffice's own ODF *exporter* writes a multi-section format, and
no hand-authored file can be trusted to reproduce it. What it writes is one
`number:*-style` per section, linked from the one the cell names by
`style:map`, with the named style's own body as the LAST section.

The six formats, and what each separates:

  A  `0.0;[Red]-0.0;[Blue]0.0`
        three sections, so TWO maps, so both carry an explicit condition. Its
        colours are on the negative and the ZERO section, which is the one a
        reader that colours by the sign of the number gets wrong.

  B  `#,##0 ;[Red](#,##0)`
        one map at `value()>=0`, whose condition LibreOffice deliberately does
        NOT write into the code. Its two sections differ in TEXT -- `100` and
        `(100)` -- so it measures the assembly and not only the colour.

  C  `_(* #,##0_);_(* (#,##0);_(* "-"_);_(@_)`
        the accounting format. Its owner is a `number:text-style` and its three
        maps are the numeric sections, the last of which is written bare
        because "the last condition can only be all other numbers".

  D  `[>=100]"big" 0;[<0]"neg" 0;0.0`
        conditions that are not the default pair, so neither may be dropped.

  E  `[COLOR10]0.0`
        a palette index. 26.2.4.2 resolves it from the running installation's
        own `standard.soc` and its ODF export writes `fo:color="#dddddd"`,
        which is NOT one of the ten keyword colours -- so the reference itself
        drops it on re-import and draws the cell black. The control that says
        a reader must not resolve an arbitrary RGB here.

  F  `0.00`
        no map at all: the single-section path this round must not disturb.
"""
import zipfile, pathlib, subprocess, sys, shutil, tempfile

R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
SOFFICE = '/opt/libreoffice26.2/program/soffice'

FORMATS = [
    '0.0;[Red]-0.0;[Blue]0.0',
    '#,##0 ;[Red](#,##0)',
    '_(* #,##0_);_(* (#,##0);_(* &quot;-&quot;_);_(@_)',
    '[&gt;=100]&quot;big&quot; 0;[&lt;0]&quot;neg&quot; 0;0.0',
    '[COLOR10]0.0',
    '0.00',
]

# One row per value, one column per format, so a cell's address states which
# arm it is: column A..F is the format, row 1..4 is positive, negative, zero,
# text.
VALUES = [150.0, -100.0, 0.0, None]
COLUMNS = 'ABCDEF'


def parts():
    numfmts = ''.join(
        f'<numFmt numFmtId="{164 + i}" formatCode="{code}"/>' for i, code in enumerate(FORMATS))
    xfs = ''.join(
        f'<xf numFmtId="{164 + i}" applyNumberFormat="1" applyAlignment="1" xfId="0"><alignment wrapText="1"/></xf>' for i in range(len(FORMATS)))

    rows = []
    for r, value in enumerate(VALUES, start=1):
        cells = []
        for c, column in enumerate(COLUMNS):
            if value is None:
                cells.append(f'<c r="{column}{r}" s="{c}" t="inlineStr">'
                             f'<is><t>text</t></is></c>')
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
<cols><col min="1" max="6" width="6" customWidth="1"/></cols>
<sheetData>
''' + '\n'.join(rows) + '''
</sheetData>
</worksheet>''',
    }


def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1
                       else 'dotnet/tests/corpus/features/sheet-odf-numfmt-sections.ods')
    out.parent.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as work:
        work = pathlib.Path(work)
        book = work / 'sheet-odf-numfmt-sections.xlsx'
        with zipfile.ZipFile(book, 'w', zipfile.ZIP_DEFLATED) as z:
            for name, text in parts().items():
                z.writestr(name, text)

        subprocess.run(
            [SOFFICE, '--headless', f'-env:UserInstallation=file://{work}/profile',
             '--convert-to', 'ods', '--outdir', str(work), str(book)],
            check=True, capture_output=True, timeout=300)

        made = work / 'sheet-odf-numfmt-sections.ods'
        if not made.exists():
            raise SystemExit('conversion produced nothing -- assert the instrument ran')
        shutil.copy(made, out)

    print(f'wrote {out} ({out.stat().st_size} bytes)')


if __name__ == '__main__':
    main()
