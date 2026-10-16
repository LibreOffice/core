#!/usr/bin/env python3
"""Author `tests/corpus/features/sheet-band-scale-xlsx.xlsx`.

Three worksheets asking one question each about the print scale reaching a header band:

  * `Unscaled`  -- scale 100, the CONTROL. Nothing about this fixture may move it.
  * `Scaled`    -- scale 40, the same band and the same body cell. The body's origin must be
                   `headerMargin + bandHeight * 0.40`, not `headerMargin + bandHeight`.
  * `Pinned`    -- scale 50 over a band whose text does not fit (three 11 pt lines in 32.4 pt),
                   so the band is PINNED at the stated 32.4 and the two arms of `SheetBandHeight`
                   are separated: a pinned band scales exactly as a dynamic one does.

Letter portrait, workbook default Liberation Sans 11, one wide column so nothing wraps.
"""
import os, zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
OUT = "/c/sandbox/workdir/wt-sheets-r50/dotnet/tests/corpus/features/sheet-band-scale-xlsx.xlsx"

SHEETS = [
    # name,      scale, header string,                             body token
    ("Unscaled", 100, "&C&14ZZTOPZZ", "ZZBODY1ZZ"),
    ("Scaled", 40, "&C&14ZZTOPZZ", "ZZBODY2ZZ"),
    ("Pinned", 50, "&CZZPIN1ZZ\nZZPIN2ZZ\nZZPIN3ZZ", "ZZBODY3ZZ"),
]


def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;")


def sheet_xml(scale, header, body):
    return (
        f'<worksheet xmlns="{NS}" xmlns:r="{RNS}">'
        '<cols><col min="1" max="1" width="30" customWidth="1"/></cols>'
        f'<sheetData><row r="1"><c r="A1" t="inlineStr"><is><t>{body}</t></is></c></row></sheetData>'
        '<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75" header="0.3" footer="0.3"/>'
        f'<pageSetup paperSize="1" scale="{scale}" orientation="portrait"/>'
        f'<headerFooter><oddHeader>{esc(header)}</oddHeader></headerFooter>'
        '</worksheet>')


def main():
    styles = (
        f'<styleSheet xmlns="{NS}">'
        '<fonts count="1"><font><sz val="11"/><name val="Liberation Sans"/></font></fonts>'
        '<fills count="2"><fill><patternFill patternType="none"/></fill>'
        '<fill><patternFill patternType="gray125"/></fill></fills>'
        '<borders count="1"><border/></borders>'
        '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
        '<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/></cellXfs>'
        '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
        '<dxfs count="0"/></styleSheet>')

    types = ['<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
             '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
             '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
             '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>']
    rels = ['<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">']
    sheets = []
    for i, (name, scale, header, body) in enumerate(SHEETS, start=1):
        types.append(f'<Override PartName="/xl/worksheets/sheet{i}.xml"'
                     ' ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>')
        rels.append(f'<Relationship Id="rId{i}" Type="{RNS}/worksheet" Target="worksheets/sheet{i}.xml"/>')
        sheets.append(f'<sheet name="{name}" sheetId="{i}" r:id="rId{i}"/>')
    rels.append(f'<Relationship Id="rIdS" Type="{RNS}/styles" Target="styles.xml"/>')
    types.append("</Types>")
    rels.append("</Relationships>")

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with zipfile.ZipFile(OUT, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", "".join(types))
        z.writestr("_rels/.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/officeDocument" Target="xl/workbook.xml"/></Relationships>')
        z.writestr("xl/_rels/workbook.xml.rels", "".join(rels))
        z.writestr("xl/workbook.xml",
            f'<workbook xmlns="{NS}" xmlns:r="{RNS}"><sheets>{"".join(sheets)}</sheets></workbook>')
        z.writestr("xl/styles.xml", styles)
        for i, (name, scale, header, body) in enumerate(SHEETS, start=1):
            z.writestr(f"xl/worksheets/sheet{i}.xml", sheet_xml(scale, header, body))
    print(OUT)


if __name__ == "__main__":
    main()
