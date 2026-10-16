#!/usr/bin/env python3
"""Author the five conditional-format fixtures under dotnet/tests/corpus/features/.

One file per rule family this round implements, each the smallest workbook that still shows the
behaviour, each with a unique stem — `soffice --convert-to` names its output after the stem alone
and two files sharing one silently overwrite each other.

Every part a real writer emits is present: [Content_Types].xml, both _rels parts, workbook.xml,
styles.xml with a `dxfs` table, sharedStrings.xml and one worksheet. A hand-built fixture missing
a part the importer takes its defaults from answers a different question — see the
`office-corpus` skill.

Usage: make-fixtures.py <outdir>
"""
import sys
import pathlib
import zipfile

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

WB_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>"""

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"

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


def sheet(cells, rules, strings):
    """cells: {(row1based, colLetter): value}, a str goes to the shared table, a number stays."""
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
            f'<worksheet xmlns="{NS}"><sheetData>{data}</sheetData>{rules}</worksheet>')


def shared(strings):
    items = "".join(f'<si><t xml:space="preserve">{s}</t></si>' for s in strings)
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<sst xmlns="{NS}" count="{len(strings)}" uniqueCount="{len(strings)}">{items}</sst>')


def write(path, cells, rules, dxfs):
    strings = []
    body = sheet(cells, rules, strings)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("xl/workbook.xml", WB)
        z.writestr("xl/_rels/workbook.xml.rels", WB_RELS)
        z.writestr("xl/styles.xml", styles(dxfs))
        z.writestr("xl/sharedStrings.xml", shared(strings))
        z.writestr("xl/worksheets/sheet1.xml", body)


# One red fill with a red font, which is what every corpus rule of these families names.
RED = ('<dxf><font><color rgb="FF9C0006"/></font>'
       '<fill><patternFill><bgColor rgb="FFFFC7CE"/></patternFill></fill></dxf>')
# A second, distinguishable, so a precedence assertion can name which rule won.
BLUE = ('<dxf><font><color rgb="FF0000FF"/></font>'
        '<fill><patternFill><bgColor rgb="FFCCE5FF"/></patternFill></fill></dxf>')


def rule(kind, sqref, dxf=0, priority=1, text=None, operator=None, body=""):
    t = f' text="{text}"' if text is not None else ""
    o = f' operator="{operator}"' if operator else ""
    return (f'<conditionalFormatting sqref="{sqref}">'
            f'<cfRule type="{kind}"{o} dxfId="{dxf}" priority="{priority}"{t}>{body}</cfRule>'
            f'</conditionalFormatting>')


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)

    # 1. containsText. A1 holds the needle outright, A2 in the middle of a longer string, A3 in a
    #    different case (Calc is case-insensitive here by default), A4 not at all, A5 is a number
    #    whose digits hold it — the arm `IsValid` takes, which does *not* lowercase — and A6 is
    #    empty.
    write(out / "sheet-cf-contains-text.xlsx",
          {(1, "A"): "Open", (2, "A"): "Reopened later", (3, "A"): "OPEN",
           (4, "A"): "Closed", (5, "A"): 1924, (6, "A"): "Rope"},
          rule("containsText", "A1:A6", 0, 1, text="ope", operator="containsText",
               body='<formula>NOT(ISERROR(SEARCH("ope",A1)))</formula>'),
          [RED])

    # 2. endsWith. A1 ends with it, A2 holds it in the middle only, A3 ends with it in another
    #    case, A4 is shorter than the needle.
    write(out / "sheet-cf-ends-with.xlsx",
          {(1, "A"): "Case Closed", (2, "A"): "Closed case", (3, "A"): "case CLOSED",
           (4, "A"): "no"},
          rule("endsWith", "A1:A4", 0, 1, text="closed", operator="endsWith",
               body='<formula>RIGHT(A1,LEN("closed"))="closed"</formula>'),
          [RED])

    # 3. containsBlanks and notContainsBlanks, on the same column, so one fixture shows both and
    #    the pair partitions the range. A2 holds three spaces, which is the whole question:
    #    LibreOffice's replacement formula is `LEN(TRIM(#B))=0` and trims them away.
    write(out / "sheet-cf-blank-cells.xlsx",
          {(1, "A"): "text", (2, "A"): "   ", (4, "A"): 0, (5, "A"): "x"},
          rule("containsBlanks", "A1:A5", 0, 1)
          + rule("notContainsBlanks", "A1:A5", 1, 2),
          [RED, BLUE])

    # 4. duplicateValues. A1/A2 are the same string, A3 the same string in another case — the
    #    cache is keyed on the lowercased string — A4 unique, A5/A6 the same number, and A7 the
    #    same digits as a string, which is a different key and therefore not a duplicate of them.
    write(out / "sheet-cf-duplicate-values.xlsx",
          {(1, "A"): "alpha", (2, "A"): "alpha", (3, "A"): "ALPHA", (4, "A"): "bravo",
           (5, "A"): 7, (6, "A"): 7, (7, "A"): "7"},
          rule("duplicateValues", "A1:A8", 0, 1),
          [RED])

    # 5. The multi-range anchor. `$B1="hit"` is stated on `B3 A5` — componentwise minimum A3,
    #    `ScRangeList::GetTopLeftCorner` A5, so the two readings shift the relative row by two and
    #    disagree about which B decides each cell.
    write(out / "sheet-cf-multi-range-anchor.xlsx",
          {(1, "B"): "hit", (3, "B"): "miss", (5, "A"): "five", (3, "A"): "three"},
          '<conditionalFormatting sqref="B3 A5">'
          '<cfRule type="expression" dxfId="0" priority="1">'
          '<formula>$B1="hit"</formula></cfRule></conditionalFormatting>',
          [RED])

    for f in sorted(out.glob("sheet-cf-*.xlsx")):
        print(f, f.stat().st_size)


if __name__ == "__main__":
    main()
