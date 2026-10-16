#!/usr/bin/env python3
"""Author the one workbook the corpus cannot supply: `cellXfs[0]` != the `Normal` cellStyleXf.

`clearContents(… HARDATTR | STYLES …)` takes a pivot cell back to Calc's **Default** cell
style, which the OOXML import builds from the `Normal` `cellStyle`'s `cellStyleXfs` entry —
*not* from `cellXfs[0]`, which is only what a cell stating no `s` resolves through. Every
workbook in the corpus gives the two the same content, so no corpus document can tell the two
apart and `probes/pivot-res-r108` shipped `cellXfs[0]` as an approximation.

This fixture states them differently and asks 26.2.4.2 which one a cleared pivot cell lands on:

  `cellStyleXfs[0]`, the `Normal` style   font 0 — Liberation Sans 11, black, no fill
  `cellXfs[0]`, the default cell format   font 1 — Liberation Serif 18, red, no fill
  `cellXfs[1]`, hard on the pivot's cells font 2 — Liberation Mono 8, bold, green, yellow fill

Three sheets so that one rendering answers three questions at once:

  `Data`    the pivot cache's source range, and nothing else.
  `Cleared` a pivot laid over cells that every one state `cellXfs[1]`. What the reference
            resolves them to after clearing is the answer.
  `Plain`   the same block three times and no pivot — the control. A1:C4 states `cellXfs[1]`,
            A7:C10 states `cellXfs[0]`, A13:C16 states no `s` at all, so all three fonts stand
            in one `.fods` and a reading of the answer cannot be a misreading of the
            instrument.

Everything but `styles.xml` and the sheet list is `probes/pivot-res-r108/make-fixture.py`'s,
which was itself checked part for part against 26.2.4.2's own output — the corpus skill's
warning is that a fixture minimal enough to be obviously right can be minimal enough to answer
a different question, so this one is a modification of a fixture already known to import.

    make-default-fixture.py <out.xlsx>
"""
import sys, zipfile

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/worksheets/sheet2.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/worksheets/sheet3.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/pivotCache/pivotCacheDefinition1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheDefinition+xml"/>
<Override PartName="/xl/pivotCache/pivotCacheRecords1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheRecords+xml"/>
<Override PartName="/xl/pivotTables/pivotTable1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
</Types>'''

ROOT_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

WORKBOOK = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets>
<sheet name="Data" sheetId="1" r:id="rId1"/>
<sheet name="Cleared" sheetId="2" r:id="rId2"/>
<sheet name="Plain" sheetId="3" r:id="rId3"/>
</sheets>
<pivotCaches><pivotCache cacheId="1" r:id="rId5"/></pivotCaches>
</workbook>'''

WB_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet2.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet3.xml"/>
<Relationship Id="rId4" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId5" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition" Target="pivotCache/pivotCacheDefinition1.xml"/>
</Relationships>'''

#  font 0 is the Normal cell style's, font 1 is cellXfs[0]'s, font 2 is the hard one on the
#  pivot's own cells.  The three differ in face, size and colour at once so that a `.fods`
#  naming any one of them is unambiguous.
STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="3"><font><sz val="11"/><color rgb="FF000000"/><name val="Liberation Sans"/></font><font><sz val="18"/><color rgb="FFFF0000"/><name val="Liberation Serif"/></font><font><b/><sz val="8"/><color rgb="FF00A000"/><name val="Liberation Mono"/></font></fonts>
<fills count="3"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill><fill><patternFill patternType="solid"><fgColor rgb="FFFFFF00"/><bgColor indexed="64"/></patternFill></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
<cellXfs count="2"><xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1"/><xf numFmtId="0" fontId="2" fillId="2" borderId="0" xfId="0" applyFont="1" applyFill="1"/></cellXfs>
</styleSheet>'''

DATA_ROWS = [
    ('Region', 'Product', 'Amount'),
    ('North', 'Apple', 10), ('North', 'Pear', 20),
    ('South', 'Apple', 30), ('South', 'Pear', 40),
]

HIDDEN_ROWS = [
    ('Region', 'Sum of Amount', 'Count of Amount'),
    ('North', 30, 2),
    ('South', 70, 2),
    ('Grand Total', 100, 4),
]


def cell(ref, value, style):
    """`style` None omits `s` altogether, which is not the same as `s="0"`.

    `sheetdatacontext.cxx`'s `importCell` reads `s` with a default of -1 and applies no
    format at all when it is absent, so a cell that omits `s` keeps Calc's Default cell style
    where one stating `s="0"` takes `cellXfs[0]`. Every block of the control sheet below is
    there to make that distinction visible in the one `.fods`.
    """
    s = '' if style is None else ' s="%d"' % style
    if isinstance(value, (int, float)):
        return '<c r="%s"%s><v>%s</v></c>' % (ref, s, value)
    return '<c r="%s"%s t="str"><v>%s</v></c>' % (ref, s, value)


def sheet(rows, style=None):
    out = []
    for r, values in enumerate(rows, 1):
        cs = ''.join(cell('%s%d' % (chr(65 + c), r), v, style)
                     for c, v in enumerate(values) if v != '')
        out.append('<row r="%d">%s</row>' % (r, cs))
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
            '<sheetData>%s</sheetData></worksheet>' % ''.join(out))


def control_sheet():
    """The same block three times, one per state a cell can be in.

    A1:C4 states `cellXfs[1]`, the hard format the pivot's own cells carry; A7:C10 states
    `cellXfs[0]` outright; A13:C16 states no `s` at all and therefore takes Calc's Default.
    """
    out = []
    for at, style in ((1, 1), (7, 0), (13, None)):
        for r, values in enumerate(HIDDEN_ROWS, at):
            cs = ''.join(cell('%s%d' % (chr(65 + c), r), v, style)
                         for c, v in enumerate(values) if v != '')
            out.append('<row r="%d">%s</row>' % (r, cs))
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
            '<sheetData>%s</sheetData></worksheet>' % ''.join(out))


CACHE_DEF = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<pivotCacheDefinition xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" r:id="rId1" refreshOnLoad="0" recordCount="4">
<cacheSource type="worksheet"><worksheetSource ref="A1:C5" sheet="Data"/></cacheSource>
<cacheFields count="3">
<cacheField name="Region" numFmtId="0"><sharedItems count="2"><s v="North"/><s v="South"/></sharedItems></cacheField>
<cacheField name="Product" numFmtId="0"><sharedItems count="2"><s v="Apple"/><s v="Pear"/></sharedItems></cacheField>
<cacheField name="Amount" numFmtId="0"><sharedItems containsSemiMixedTypes="0" containsString="0" containsNumber="1" containsInteger="1" minValue="10" maxValue="40"/></cacheField>
</cacheFields>
</pivotCacheDefinition>'''

CACHE_DEF_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheRecords" Target="pivotCacheRecords1.xml"/>
</Relationships>'''

CACHE_RECORDS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<pivotCacheRecords xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="4">
<r><x v="0"/><x v="0"/><n v="10"/></r>
<r><x v="0"/><x v="1"/><n v="20"/></r>
<r><x v="1"/><x v="0"/><n v="30"/></r>
<r><x v="1"/><x v="1"/><n v="40"/></r>
</pivotCacheRecords>'''

CLEARED = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<pivotTableDefinition xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" name="PivotCleared" cacheId="1" dataOnRows="0" applyNumberFormats="0" applyBorderFormats="0" applyFontFormats="0" applyPatternFormats="0" applyAlignmentFormats="0" applyWidthHeightFormats="1" dataCaption="Values" updatedVersion="6" minRefreshableVersion="3" useAutoFormatting="1" itemPrintTitles="1" createdVersion="4" indent="0" compact="0" compactData="0" outline="0" outlineData="0" multipleFieldFilters="0">
<location ref="A1:C4" firstHeaderRow="0" firstDataRow="1" firstDataCol="1"/>
<pivotFields count="3">
<pivotField axis="axisRow" compact="0" outline="0" subtotalTop="0" showAll="0" defaultSubtotal="0"><items count="2"><item x="0"/><item x="1"/></items></pivotField>
<pivotField compact="0" outline="0" showAll="0"/>
<pivotField dataField="1" compact="0" outline="0" showAll="0"/>
</pivotFields>
<rowFields count="1"><field x="0"/></rowFields>
<rowItems count="3">
<i><x v="0"/></i>
<i><x v="1"/></i>
<i t="grand"><x/></i>
</rowItems>
<colFields count="1"><field x="-2"/></colFields>
<colItems count="2"><i><x/></i><i i="1"><x v="1"/></i></colItems>
<dataFields count="2"><dataField name="Sum of Amount" fld="2" baseField="0" baseItem="0"/><dataField name="Count of Amount" fld="2" subtotal="count" baseField="0" baseItem="0"/></dataFields>
</pivotTableDefinition>'''

PIVOT_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition" Target="../pivotCache/pivotCacheDefinition1.xml"/>
</Relationships>'''

SHEET_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotTable" Target="../pivotTables/pivotTable1.xml"/>
</Relationships>'''


def main():
    with zipfile.ZipFile(sys.argv[1], 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('xl/workbook.xml', WORKBOOK)
        z.writestr('xl/_rels/workbook.xml.rels', WB_RELS)
        z.writestr('xl/styles.xml', STYLES)
        z.writestr('xl/worksheets/sheet1.xml', sheet(DATA_ROWS))
        z.writestr('xl/worksheets/sheet2.xml', sheet(HIDDEN_ROWS, style=1))
        z.writestr('xl/worksheets/sheet3.xml', control_sheet())
        z.writestr('xl/worksheets/_rels/sheet2.xml.rels', SHEET_RELS)
        z.writestr('xl/pivotCache/pivotCacheDefinition1.xml', CACHE_DEF)
        z.writestr('xl/pivotCache/_rels/pivotCacheDefinition1.xml.rels', CACHE_DEF_RELS)
        z.writestr('xl/pivotCache/pivotCacheRecords1.xml', CACHE_RECORDS)
        z.writestr('xl/pivotTables/pivotTable1.xml', CLEARED)
        z.writestr('xl/pivotTables/_rels/pivotTable1.xml.rels', PIVOT_RELS)


main()
