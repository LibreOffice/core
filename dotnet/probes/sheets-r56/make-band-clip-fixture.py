#!/usr/bin/env python3
"""Authors `tests/corpus/features/sheet-band-clip-xlsx.xlsx`.

Two worksheets, one question each, and a workbook default font of **Times New Roman 14** so
that both halves of round 56's change are observable in one file — Times New Roman resolves to
Liberation Serif here, which is neither the face nor the size the band used to be drawn in.

  `Areas`  the per-area clip. A 7.2 pt band (top 0.40 in, header 0.30 in) pinned by a header
           whose LEFT area is one line and whose RIGHT area is seven empty lines and then one.
           The left area's ink starts 2.9 pt below the band's top and is inside it; the right
           area's starts about 110 pt down and is not. `PrintHF` clips both to the same
           rectangle and `DrawText_ToPosition` drops an area whose whole range misses it, so
           `KEEPLEFT` prints and `DROPRIGHT` does not. This is `FAA-2019-0995-0002_attachment_2`'s
           shape, and it is also the discriminator between per-area and per-line clipping.

  `Face`   the band's default face. A roomy 36 pt footer band with three areas: the left names
           `&"Courier New"`, the centre names nothing, and the right names `&24`. Before round
           56 all three printed in ten-point Liberation Sans.

Fixture shape is `probes/sheets-r53-totalsrow/audit_mkwb.py`'s, which 26.2.4.2 is known to read
correctly — a minimal `.xlsx` with no `<cellStyles>` has its `cellXf` font discarded entirely.
Confirm any change here by rendering it through `soffice --convert-to pdf` and reading the
positions back, which is where every figure in `SheetBandClipFixtureTests` comes from.
"""
import os, sys, zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"

SHEETS = [
    # name, margins, header string, footer string
    ("Areas",
     dict(top=0.40, bottom=0.75, header=0.30, footer=0.30),
     "&LKEEPLEFT&R\n\n\n\n\n\n\nDROPRIGHT",
     None),
    ("Face",
     dict(top=0.75, bottom=0.80, header=0.30, footer=0.30),
     None,
     '&L&"Courier New"FACECODE&CPLAINFACE&R&24BIGFACE'),
    # The two that decide *where* a line's ink starts, and they are a pair on purpose. 14 pt
    # Times New Roman puts its ink about 2.9 pt below the line's top, so a 2.0 pt band draws
    # nothing and a 4.0 pt band draws everything. A rule that took the line box's top instead of
    # the ink's would draw both; a rule that took the whole line height would draw neither.
    ("Sliver",
     dict(top=0.32778, bottom=0.75, header=0.30, footer=0.30),
     "&CTHINBAND",
     None),
    ("Slice",
     dict(top=0.35556, bottom=0.75, header=0.30, footer=0.30),
     "&CWIDEBAND",
     None),
]


def escape(s):
    return s.replace("&", "&amp;").replace("<", "&lt;")


def sheet_xml(margins, header, footer):
    hf = ""
    if header or footer:
        hf = "<headerFooter>"
        if header:
            hf += "<oddHeader>%s</oddHeader>" % escape(header)
        if footer:
            hf += "<oddFooter>%s</oddFooter>" % escape(footer)
        hf += "</headerFooter>"
    return (
        f'<worksheet xmlns="{NS}" xmlns:r="{RNS}">'
        '<sheetData><row r="1"><c r="A1" t="inlineStr"><is><t>BODYCELL</t></is></c></row></sheetData>'
        '<pageMargins left="0.7" right="0.7" top="%(top)s" bottom="%(bottom)s"'
        ' header="%(header)s" footer="%(footer)s"/>' % margins +
        '<pageSetup paperSize="1" orientation="portrait"/>'
        f'{hf}</worksheet>')


def main(path):
    styles = (
        f'<styleSheet xmlns="{NS}">'
        '<fonts count="1"><font><sz val="14"/><name val="Times New Roman"/></font></fonts>'
        '<fills count="2"><fill><patternFill patternType="none"/></fill>'
        '<fill><patternFill patternType="gray125"/></fill></fills>'
        '<borders count="1"><border/></borders>'
        '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
        '<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/></cellXfs>'
        '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
        '<dxfs count="0"/></styleSheet>')

    overrides = "".join(
        '<Override PartName="/xl/worksheets/sheet%d.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
        % (n + 1) for n in range(len(SHEETS)))
    rels = "".join(
        f'<Relationship Id="rId{n + 1}" Type="{RNS}/worksheet" Target="worksheets/sheet{n + 1}.xml"/>'
        for n in range(len(SHEETS)))
    tabs = "".join(
        '<sheet name="%s" sheetId="%d" r:id="rId%d"/>' % (s[0], n + 1, n + 1)
        for n, s in enumerate(SHEETS))

    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml",
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
            + overrides +
            '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
            '</Types>')
        z.writestr("_rels/.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/officeDocument" Target="xl/workbook.xml"/></Relationships>')
        z.writestr("xl/_rels/workbook.xml.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            + rels +
            f'<Relationship Id="rId{len(SHEETS) + 1}" Type="{RNS}/styles" Target="styles.xml"/></Relationships>')
        z.writestr("xl/workbook.xml",
            f'<workbook xmlns="{NS}" xmlns:r="{RNS}"><sheets>{tabs}</sheets></workbook>')
        z.writestr("xl/styles.xml", styles)
        for n, (_, margins, header, footer) in enumerate(SHEETS):
            z.writestr("xl/worksheets/sheet%d.xml" % (n + 1), sheet_xml(margins, header, footer))
    print("wrote", path)


main(sys.argv[1] if len(sys.argv) > 1
     else "/c/sandbox/workdir/wt-sheets-r50/dotnet/tests/corpus/features/sheet-band-clip-xlsx.xlsx")
