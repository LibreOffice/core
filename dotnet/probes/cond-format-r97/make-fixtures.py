#!/usr/bin/env python3
"""Author round 97's conditional-format fixtures under dotnet/tests/corpus/features/.

Same shape as `probes/cond-format-r96/make-fixtures.py`: the smallest workbook that still shows
the behaviour, every part a real writer emits present, and **a unique stem per file** — a
`soffice --convert-to` names its output after the stem alone, so two fixtures sharing one stem
silently overwrite each other's PDF.

Five files:

  sheet-cf-stored-vs-drawn      the compared value is the stored string, not the drawn one
  sheet-cf-data-bar-lengths     a `dataBar` with no `x14` extension: minLength/maxLength apply
  sheet-cf-data-bar-auto        the same rule with the extension: they do not
  sheet-cf-data-bar-negative    the zero position, the negative colour and the axis
  sheet-cf-data-bar-only        `showValue="0"` takes the cell's own text off the page

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
<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>"""

WB_RELS = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="{R}/styles" Target="styles.xml"/>
<Relationship Id="rId3" Type="{R}/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>"""

WB = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="{NS}" xmlns:r="{R}"><sheets>
<sheet name="Rules" sheetId="1" r:id="rId1"/></sheets></workbook>"""


def styles(dxfs):
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="{NS}">
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="2"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>
<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
<dxfs count="{len(dxfs)}">{''.join(dxfs)}</dxfs>
</styleSheet>"""


def sheet(cells, rules, strings, tail=""):
    """cells: {(row1based, colLetter): value}; a str goes to the shared table, a number stays.

    A string is written to `sharedStrings.xml` verbatim, so an `_xHHHH_` escape in it is an
    escape in the file — which is the point of the first fixture.
    """
    rows = {}
    for (row, col), value in sorted(cells.items(), key=lambda kv: (kv[0][0], kv[0][1])):
        if isinstance(value, str):
            if value not in strings:
                strings.append(value)
            body = f'<c r="{col}{row}" t="s"><v>{strings.index(value)}</v></c>'
        else:
            body = f'<c r="{col}{row}"><v>{value}</v></c>'
        rows.setdefault(row, []).append(body)
    data = "".join(f'<row r="{r}">{"".join(cs)}</row>' for r, cs in sorted(rows.items()))
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<worksheet xmlns="{NS}"><sheetData>{data}</sheetData>{rules}{tail}</worksheet>')


def shared(strings):
    items = "".join(f'<si><t xml:space="preserve">{s}</t></si>' for s in strings)
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<sst xmlns="{NS}" count="{len(strings)}" uniqueCount="{len(strings)}">{items}</sst>')


def write(path, cells, rules, dxfs, tail=""):
    strings = []
    body = sheet(cells, rules, strings, tail)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("xl/workbook.xml", WB)
        z.writestr("xl/_rels/workbook.xml.rels", WB_RELS)
        z.writestr("xl/styles.xml", styles(dxfs))
        z.writestr("xl/sharedStrings.xml", shared(strings))
        z.writestr("xl/worksheets/sheet1.xml", body)


RED = ('<dxf><font><color rgb="FF9C0006"/></font>'
       '<fill><patternFill><bgColor rgb="FFFFC7CE"/></patternFill></fill></dxf>')
BLUE = ('<dxf><font><color rgb="FF0000FF"/></font>'
        '<fill><patternFill><bgColor rgb="FFCCE5FF"/></patternFill></fill></dxf>')

# A bar colour no other ink on the page uses, so a rectangle census can name the bars without
# having to know where the cells are.
BAR = "FF2E75B6"
NEGATIVE = "FFC00000"


def rule(kind, sqref, dxf=0, priority=1, text=None, operator=None, body=""):
    t = f' text="{text}"' if text is not None else ""
    o = f' operator="{operator}"' if operator else ""
    d = f' dxfId="{dxf}"' if dxf is not None else ""
    return (f'<conditionalFormatting sqref="{sqref}">'
            f'<cfRule type="{kind}"{o}{d} priority="{priority}"{t}>{body}</cfRule>'
            f'</conditionalFormatting>')


def data_bar(sqref, cfvos, colour=BAR, show_value=None, min_length=None, max_length=None,
             ext=None, priority=1):
    """One `dataBar` block; `ext` is the `x14:id` GUID when the file also states an extension."""
    attrs = ""
    if show_value is not None:
        attrs += f' showValue="{show_value}"'
    if min_length is not None:
        attrs += f' minLength="{min_length}"'
    if max_length is not None:
        attrs += f' maxLength="{max_length}"'
    entries = "".join(
        f'<cfvo type="{t}"/>' if v is None else f'<cfvo type="{t}" val="{v}"/>'
        for t, v in cfvos)
    tail = ''
    if ext:
        tail = ('<extLst><ext uri="{B025F937-C7B1-47D3-B67F-A62EFF666E3E}" '
                f'xmlns:x14="{X14}"><x14:id>{ext}</x14:id></ext></extLst>')
    return (f'<conditionalFormatting sqref="{sqref}">'
            f'<cfRule type="dataBar" priority="{priority}">'
            f'<dataBar{attrs}>{entries}<color rgb="{colour}"/></dataBar>{tail}'
            f'</cfRule></conditionalFormatting>')


def x14_data_bar(guid, sqref, cfvos, negative=None, axis_position=None, gradient="0"):
    """The `x14` half, in the worksheet's own `extLst`, which is what turns the axis on."""
    pos = f' axisPosition="{axis_position}"' if axis_position else ""
    entries = "".join(
        f'<x14:cfvo type="{t}"/>' if v is None
        else f'<x14:cfvo type="{t}"><xm:f>{v}</xm:f></x14:cfvo>'
        for t, v in cfvos)
    neg = f'<x14:negativeFillColor rgb="{negative}"/>' if negative else ""
    return (f'<extLst><ext uri="{{78C0D931-6437-407d-A8EE-F0AAD7539E65}}" xmlns:x14="{X14}">'
            f'<x14:conditionalFormattings><x14:conditionalFormatting xmlns:xm="{XM}">'
            f'<x14:cfRule type="dataBar" id="{guid}">'
            f'<x14:dataBar minLength="0" maxLength="100" gradient="{gradient}"{pos}>'
            f'{entries}{neg}<x14:axisColor rgb="FF000000"/></x14:dataBar>'
            f'</x14:cfRule><xm:sqref>{sqref}</xm:sqref>'
            f'</x14:conditionalFormatting></x14:conditionalFormattings></ext></extLst>')


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)

    # 1. The stored string against the drawn one. Every `_x0009_` is a tab Calc keeps in the cell
    #    and draws as nothing, so the drawn text of A1 and A2 is the same and their stored text is
    #    not. A duplicate rule keys on the stored one, and the blank rule's `LEN(TRIM(#B))=0`
    #    measures the stored one too — TRIM takes spaces and not tabs.
    write(out / "sheet-cf-stored-vs-drawn.xlsx",
          {(1, "A"): "_x0009_alpha", (2, "A"): "alpha", (3, "A"): "alpha",
           (4, "A"): "bravo", (5, "A"): "_x0009_bravo", (6, "A"): "_x0009_bravo",
           (1, "C"): "_x0009_", (2, "C"): "   ", (3, "C"): "zulu"},
          rule("duplicateValues", "A1:A6", 0, 1)
          + rule("containsBlanks", "C1:C3", 0, 2)
          + rule("notContainsBlanks", "C1:C3", 1, 3),
          [RED, BLUE])

    # 2. A `dataBar` with no `x14` extension. `DataBarRule`'s constructor leaves the axis at
    #    `databar::NONE`, which is the one arm of `GetDataBarInfo` that uses `minLength` and
    #    `maxLength` — defaulted to 10 and 90 by `importAttribs`.
    write(out / "sheet-cf-data-bar-lengths.xlsx",
          {(1, "A"): 0, (2, "A"): 25, (3, "A"): 50, (4, "A"): 75, (5, "A"): 100},
          data_bar("A1:A5", [("min", None), ("max", None)]),
          [])

    # 3. The same rule with the extension present and no `axisPosition` on it, which is what all
    #    nine corpus rules look like. The axis becomes `AUTOMATIC`, the two lengths stop being
    #    read at all, and a cell at the minimum draws no bar.
    guid = "{2D9F4E31-1111-4D3B-9F0A-0000000000A1}"
    write(out / "sheet-cf-data-bar-auto.xlsx",
          {(1, "A"): 0, (2, "A"): 25, (3, "A"): 50, (4, "A"): 75, (5, "A"): 100},
          data_bar("A1:A5", [("min", None), ("max", None)], ext=guid),
          [],
          x14_data_bar(guid, "A1:A5", [("autoMin", None), ("autoMax", None)]))

    # 4. Negative values. The zero position is `-100*nMin/(nMax-nMin)` of the cell, a negative
    #    value paints leftwards from it in the negative colour, and a non-zero zero draws the
    #    dashed axis.
    guid = "{2D9F4E31-2222-4D3B-9F0A-0000000000A2}"
    write(out / "sheet-cf-data-bar-negative.xlsx",
          {(1, "A"): -100, (2, "A"): -50, (3, "A"): 0, (4, "A"): 50, (5, "A"): 100},
          data_bar("A1:A5", [("num", "-100"), ("num", "100")], ext=guid),
          [],
          x14_data_bar(guid, "A1:A5", [("num", "-100"), ("num", "100")], negative=NEGATIVE))

    # 4b. The same rule with NO `x14:negativeFillColor`. `mbNeg` is `ScDataBarFormatData`'s own
    #     default of true and no OOXML path clears it, so the negative cells take the source's
    #     `COL_LIGHTRED` fallback rather than the positive colour. 26.2.4.2 paints them #ff0000.
    #     The corpus cannot witness this — 8 of its 9 rules state no negative colour and none of
    #     the 75 cells they cover is negative — so the fixture is the only witness there is.
    guid = "{2D9F4E31-3333-4D3B-9F0A-0000000000A3}"
    write(out / "sheet-cf-data-bar-default-negative.xlsx",
          {(1, "A"): -100, (2, "A"): -50, (3, "A"): 0, (4, "A"): 50, (5, "A"): 100},
          data_bar("A1:A5", [("num", "-100"), ("num", "100")], ext=guid),
          [],
          x14_data_bar(guid, "A1:A5", [("num", "-100"), ("num", "100")]))

    # 5. `showValue="0"`. The rule paints its bar and the cell's own number is not drawn.
    write(out / "sheet-cf-data-bar-only.xlsx",
          {(1, "A"): 20, (2, "A"): 40, (3, "A"): 60, (4, "A"): 80, (5, "A"): 100},
          data_bar("A1:A5", [("min", None), ("max", None)], show_value="0"),
          [])

    for f in sorted(out.glob("sheet-cf-*.xlsx")):
        print(f, f.stat().st_size)


if __name__ == "__main__":
    main()
