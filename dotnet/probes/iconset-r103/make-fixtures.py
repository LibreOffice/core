#!/usr/bin/env python3
"""Author round 103's iconSet fixtures under dotnet/tests/corpus/features/.

Same skeleton as `probes/cond-format-r97/make-fixtures.py`: the smallest workbook that still
shows the behaviour, and **a unique stem per file** — `soffice --convert-to` names its output
after the stem alone, so two fixtures sharing one silently overwrite each other's PDF. All four
stems below were checked against `dotnet/tests/corpus`, `/home/user/sample-files` and
`/home/user/corpus-odf`: zero occurrences each.

Four files, and between them every one of the seven glyphs this tree paints:

  sheet-cf-icon-set-symbols     a main-namespace rule stating **no optional attribute at all**
  sheet-cf-icon-set-x14-custom  an `x14`-only rule: `custom`, `NoIcons` and `showValue="0"`
  sheet-cf-icon-set-reverse     `reverse="1"` composed with a `gte="0"` entry
  sheet-cf-icon-set-formula     a threshold that is a reference to another cell

The first exists because an earlier round on this family shipped an inverted arm and caught it
only on re-reading, because every one of its fixtures stated the optional attribute that most
corpus documents omit. `sheet-cf-icon-set-symbols` states `iconSet` and three `cfvo` and nothing
else — no `showValue`, no `reverse`, no `custom`, no `cfIcon`, no `gte`, no `x14` extension — so
each of those defaults is exercised by its absence.

Usage: make-fixtures.py <outdir>
"""
import pathlib
import sys
import zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
X14 = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main"
XM = "http://schemas.microsoft.com/office/excel/2006/main"

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>"""

WB_RELS = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="{R}/styles" Target="styles.xml"/>
</Relationships>"""

WB = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="{NS}" xmlns:r="{R}"><sheets>
<sheet name="Icons" sheetId="1" r:id="rId1"/></sheets></workbook>"""

# One font at eleven points, because the icon is drawn at the cell's own font height and a
# fixture that states no size would measure the application's default rather than the file's.
STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="{NS}">
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="2"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>
<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
<dxfs count="0"/>
</styleSheet>"""


def write(path, values, rules="", tail=""):
    """One column of numbers in A, with whatever conditional formatting the caller states."""
    data = "".join(
        f'<row r="{i + 1}"><c r="A{i + 1}"><v>{v}</v></c></row>'
        for i, v in enumerate(values))
    body = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<worksheet xmlns="{NS}"><sheetData>{data}</sheetData>{rules}{tail}</worksheet>')
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("xl/workbook.xml", WB)
        z.writestr("xl/_rels/workbook.xml.rels", WB_RELS)
        z.writestr("xl/styles.xml", STYLES)
        z.writestr("xl/worksheets/sheet1.xml", body)


def icon_set(sqref, entries, name=None, show_value=None, reverse=None, icons=None, priority=1):
    """A main-namespace `iconSet` rule; every keyword left None is left out of the markup."""
    attrs = ""
    if name is not None:
        attrs += f' iconSet="{name}"'
    if show_value is not None:
        attrs += f' showValue="{show_value}"'
    if reverse is not None:
        attrs += f' reverse="{reverse}"'
    if icons is not None:
        attrs += ' custom="1"'

    body = ""
    for kind, value, gte in entries:
        g = "" if gte is None else f' gte="{gte}"'
        body += f'<cfvo type="{kind}" val="{value}"{g}/>'
    for who, index in (icons or []):
        body += f'<cfIcon iconSet="{who}" iconId="{index}"/>'

    return (f'<conditionalFormatting sqref="{sqref}">'
            f'<cfRule type="iconSet" priority="{priority}">'
            f'<iconSet{attrs}>{body}</iconSet></cfRule></conditionalFormatting>')


def x14_icon_set(guid, sqref, entries, name, show_value=None, icons=None):
    """The whole rule in the worksheet's own `extLst`, with no main-namespace partner.

    This is what eighteen of the corpus's twenty rules look like, and `ExtLstLocalContext`
    imports it as a rule of its own — "an ext entry does not need to have an existing
    corresponding entry", `sc/source/filter/oox/extlstcontext.cxx`:165-194.
    """
    attrs = f' iconSet="{name}"'
    if show_value is not None:
        attrs += f' showValue="{show_value}"'
    if icons is not None:
        attrs += ' custom="1"'

    body = ""
    for entry in entries:
        kind, value = entry[0], entry[1]
        gte = "" if len(entry) < 3 or entry[2] is None else f' gte="{entry[2]}"'
        body += f'<x14:cfvo type="{kind}"{gte}><xm:f>{value}</xm:f></x14:cfvo>'
    body += "".join(
        f'<x14:cfIcon iconSet="{who}" iconId="{index}"/>' for who, index in (icons or []))

    return (f'<extLst><ext uri="{{78C0D931-6437-407d-A8EE-F0AAD7539E65}}" xmlns:x14="{X14}">'
            f'<x14:conditionalFormattings><x14:conditionalFormatting xmlns:xm="{XM}">'
            f'<x14:cfRule type="iconSet" priority="1" id="{guid}">'
            f'<x14:iconSet{attrs}>{body}</x14:iconSet>'
            f'</x14:cfRule><xm:sqref>{sqref}</xm:sqref>'
            f'</x14:conditionalFormatting></x14:conditionalFormattings></ext></extLst>')


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)

    # 1. Nothing optional stated. `3Symbols` over 0, 25, 50, 75, 100 with percent stops at 0, 33
    #    and 67: the range's own minimum and maximum are 0 and 100, so the thresholds are 0, 33
    #    and 67, every entry compares with `>=` because no `gte` says otherwise, and the *last*
    #    entry that holds wins. Cross, cross, exclamation, tick, tick — and every value is still
    #    drawn, because `showValue` defaults to true.
    write(out / "sheet-cf-icon-set-symbols.xlsx",
          [0, 25, 50, 75, 100],
          icon_set("A1:A5",
                   [("percent", 0, None), ("percent", 33, None), ("percent", 67, None)],
                   name="3Symbols"))

    # 2. An `x14`-only rule, which is the corpus's dominant spelling, with the corpus's dominant
    #    shape: `custom="1"`, a `NoIcons` first bucket and `showValue="0"`. Over -1, 0, 0.5, 1, 2
    #    with stops at percent 0 (the range minimum, -1), num 0 and num 1:
    #      -1  -> bucket 0 -> NoIcons  -> no icon *and the number is still drawn*
    #      0   -> bucket 1 -> 3Signs 0 -> a diamond, and no number
    #      0.5 -> bucket 1 -> a diamond
    #      1   -> bucket 2 -> 3Flags 0 -> a red flag
    #      2   -> bucket 2 -> a red flag
    write(out / "sheet-cf-icon-set-x14-custom.xlsx",
          [-1, 0, 0.5, 1, 2],
          tail=x14_icon_set(
              "{5E1C2A70-1103-4B0E-9D2C-000000000103}", "A1:A5",
              [("percent", 0), ("num", 0), ("num", 1)],
              name="3TrafficLights1", show_value="0",
              icons=[("NoIcons", 0), ("3Signs", 0), ("3Flags", 0)]))

    # 3. `reverse="1"` and a `gte="0"`, which compose: the bucket is found first and reflected
    #    across the entries afterwards (`colorscale.cxx`:1226-1230). Over 0, 5, 7, 10, 12 with
    #    stops at percent 0 (= 0), num 5 compared with a strict `>`, and num 10:
    #      0  -> bucket 0 -> reflected to 2 -> a green flag
    #      5  -> bucket 0 (5 > 5 is false)  -> a green flag
    #      7  -> bucket 1 -> reflected to 1 -> an amber flag
    #      10 -> bucket 2 -> reflected to 0 -> a red flag
    #      12 -> bucket 2 -> a red flag
    write(out / "sheet-cf-icon-set-reverse.xlsx",
          [0, 5, 7, 10, 12],
          icon_set("A1:A5",
                   [("percent", 0, None), ("num", 5, "0"), ("num", 10, None)],
                   name="3Flags", reverse="1"))

    # 4. A threshold that is a cell reference, which is what the corpus's only formula entries
    #    are (`$C$11` and `$D$11` in `069_Blue_modern_balance_sheet`, whose two icons are the
    #    whole of that document's icon ink). A7 holds 3; the rule's second and third stops both
    #    name it and the third compares strictly. Over 1, 2, 3, 4, 5 the range minimum is 1, so
    #    the thresholds are 1, 3 and 3 and the buckets are 0, 0, 1, 2, 2 — red, red, amber,
    #    green, green. **An unresolved reference would read as zero**, every value would clear
    #    every stop, and all five would be green: the fixture discriminates.
    write(out / "sheet-cf-icon-set-formula.xlsx",
          [1, 2, 3, 4, 5, 0, 3],
          tail=x14_icon_set(
              "{5E1C2A70-1103-4B0E-9D2C-000000000104}", "A1:A5",
              [("percent", 0), ("num", "$A$7"), ("num", "$A$7", "0")],
              name="3Flags"))

    for f in sorted(out.glob("sheet-cf-icon-set-*.xlsx")):
        print(f, f.stat().st_size)


if __name__ == "__main__":
    main()
