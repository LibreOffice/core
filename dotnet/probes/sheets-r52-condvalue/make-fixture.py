#!/usr/bin/env python3
"""Authors sheet-hidden-values-xlsx.xlsx, one worksheet per shape of the value-hiding rule.

Everything the tests assert about this file is read out of LibreOffice 26.2.4.2's own PDF of
it, not out of this script. The script only has to produce a file both readers accept.
"""
import sys, zipfile

NS = 'xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
X14 = 'xmlns:x14="http://schemas.microsoft.com/office/spreadsheetml/2009/9/main"'
XM = 'xmlns:xm="http://schemas.microsoft.com/office/excel/2006/main"'

STRINGS = ["CUSTOMROW", "PLAINROW", "PLAINSTRING", "SHOWNROW", "BARROW", "GTEROW"]


def cell(ref, value, shared=False):
    if shared:
        return f'<c r="{ref}" t="s"><v>{STRINGS.index(value)}</v></c>'
    return f'<c r="{ref}"><v>{value}</v></c>'


def row(index, cells):
    return f'<row r="{index}">' + "".join(cells) + "</row>"


def sheet(rows, body_cf="", ext_cf=""):
    ext = ""
    if ext_cf:
        ext = ('<extLst><ext uri="{78C0D931-6437-407d-A8EE-F0AAD7539E65}" ' + X14 + '>'
               '<x14:conditionalFormattings>' + ext_cf
               + '</x14:conditionalFormattings></ext></extLst>')
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<worksheet {NS}><sheetData>' + "".join(rows) + "</sheetData>"
            + body_cf + ext + "</worksheet>")


def x14_iconset(sqref, thresholds, icons, show_value="0", custom="1", reverse=None,
                icon_set="3Flags"):
    cfvo = "".join(
        f'<x14:cfvo type="{t}"{"" if g is None else f" gte=\"{g}\""}><xm:f>{v}</xm:f></x14:cfvo>'
        for t, v, g in thresholds)
    icon = "".join(f'<x14:cfIcon iconSet="{s}" iconId="{i}"/>' for s, i in icons)
    rev = "" if reverse is None else f' reverse="{reverse}"'
    return (f'<x14:conditionalFormatting {XM}><x14:cfRule type="iconSet" priority="1" '
            f'id="{{00000000-0000-0000-0000-00000000000{len(sqref)%10}}}">'
            f'<x14:iconSet iconSet="{icon_set}" showValue="{show_value}" custom="{custom}"{rev}>'
            + cfvo + icon + '</x14:iconSet></x14:cfRule>'
            f'<xm:sqref>{sqref}</xm:sqref></x14:conditionalFormatting>')


def plain_iconset(sqref, thresholds, show_value=None, icon_set="3Arrows"):
    cfvo = "".join(f'<cfvo type="{t}" val="{v}"/>' for t, v in thresholds)
    show = "" if show_value is None else f' showValue="{show_value}"'
    return (f'<conditionalFormatting sqref="{sqref}"><cfRule type="iconSet" priority="1">'
            f'<iconSet iconSet="{icon_set}"{show}>' + cfvo
            + '</iconSet></cfRule></conditionalFormatting>')


def plain_databar(sqref, show_value="0"):
    return (f'<conditionalFormatting sqref="{sqref}"><cfRule type="dataBar" priority="1">'
            f'<dataBar showValue="{show_value}"><cfvo type="min"/><cfvo type="max"/>'
            '<color rgb="FF638EC6"/></dataBar></cfRule></conditionalFormatting>')


SHEETS = []

# 1. Custom icon vector whose low band is NoIcons: the 077 shape. 11 and 22 fall in band 0 and
#    keep their text; 33 and 44 reach a real icon and lose it.
SHEETS.append(("Custom", sheet(
    [row(1, [cell("A1", "CUSTOMROW", shared=True), cell("B1", 11), cell("C1", 22),
             cell("D1", 33), cell("E1", 44)])],
    ext_cf=x14_iconset("B1:E1",
                       [("percent", 0, None), ("num", 30, None), ("num", 40, None)],
                       [("NoIcons", 0), ("3Flags", 0), ("3Signs", 0)]))))

# 2. A plain, non-custom icon set with the value hidden: every numeric cell it covers loses its
#    text and the string cell in the same range keeps it.
SHEETS.append(("Plain", sheet(
    [row(1, [cell("A1", "PLAINROW", shared=True), cell("B1", 55), cell("C1", 66),
             cell("D1", 77), cell("E1", "PLAINSTRING", shared=True)])],
    body_cf=plain_iconset("B1:E1",
                          [("percent", 0), ("percent", 33), ("percent", 67)], show_value="0"))))

# 3. The same rule with showValue absent — the control. Nothing is hidden.
SHEETS.append(("Shown", sheet(
    [row(1, [cell("A1", "SHOWNROW", shared=True), cell("B1", 88), cell("C1", 99)])],
    body_cf=plain_iconset("B1:C1", [("percent", 0), ("percent", 33), ("percent", 67)]))))

# 4. A data bar with the value hidden.
SHEETS.append(("Bar", sheet(
    [row(1, [cell("A1", "BARROW", shared=True), cell("B1", 123), cell("C1", 456)])],
    body_cf=plain_databar("B1:C1"))))

# 5. gte="0" on the middle threshold turns its boundary from >= into >, so the value that sits
#    exactly on it stays in the NoIcons band and keeps its text.
SHEETS.append(("Gte", sheet(
    [row(1, [cell("A1", "GTEROW", shared=True), cell("B1", 50), cell("C1", 51)])],
    ext_cf=x14_iconset("B1:C1",
                       [("percent", 0, None), ("num", 50, 0), ("num", 999, None)],
                       [("NoIcons", 0), ("3Flags", 0), ("3Signs", 0)]))))


def build(path):
    types = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
             '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
             '<Default Extension="xml" ContentType="application/xml"/>'
             '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
             '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
             '<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>']
    sheets_xml = []
    rels = []
    for i, (name, _) in enumerate(SHEETS, start=1):
        types.append(f'<Override PartName="/xl/worksheets/sheet{i}.xml" '
                     'ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>')
        sheets_xml.append(f'<sheet name="{name}" sheetId="{i}" r:id="rId{i}"/>')
        rels.append(f'<Relationship Id="rId{i}" '
                    'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" '
                    f'Target="worksheets/sheet{i}.xml"/>')
    n = len(SHEETS)
    rels.append(f'<Relationship Id="rId{n+1}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>')
    rels.append(f'<Relationship Id="rId{n+2}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>')
    types.append("</Types>")

    workbook = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                f'<workbook {NS}><sheets>' + "".join(sheets_xml) + "</sheets></workbook>")
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<styleSheet {NS}>'
              '<fonts count="1"><font><sz val="11"/><name val="Liberation Sans"/></font></fonts>'
              '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
              '<borders count="1"><border/></borders>'
              '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
              '<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>'
              '</styleSheet>')
    shared = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<sst {NS} count="{len(STRINGS)}" uniqueCount="{len(STRINGS)}">'
              + "".join(f"<si><t>{s}</t></si>" for s in STRINGS) + "</sst>")

    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", "".join(types))
        z.writestr("_rels/.rels",
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>'
                   "</Relationships>")
        z.writestr("xl/workbook.xml", workbook)
        z.writestr("xl/_rels/workbook.xml.rels",
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   + "".join(rels) + "</Relationships>")
        z.writestr("xl/styles.xml", styles)
        z.writestr("xl/sharedStrings.xml", shared)
        for i, (_, xml) in enumerate(SHEETS, start=1):
            z.writestr(f"xl/worksheets/sheet{i}.xml", xml)


if __name__ == "__main__":
    build(sys.argv[1])
    print(sys.argv[1])
