#!/usr/bin/env python3
"""Author a workbook whose pivot states `<format>` records, one per arm that decides a document.

`ScDPOutput::Output` clears the output range and then calls `maFormatOutput.apply`
(`dpoutput.cxx`:1190), which lays the pivot's own records over the generated styles. This is the
sibling of `make-default-fixture.py`: the same three fonts and the same hidden-header pivot, with
a `<formats>` element added and nothing else changed, so the pair is a single-variable experiment
on the whole subsystem — the control shows the clearing bare and this one shows what the records
put back.

Five records, chosen because each is an arm the corpus turns on and three of them are arms a
reading of the schema gets wrong:

  0  `<pivotArea type="all" dataOnly="0"/>` with an 18 pt font.  **Inert.** The kind is Data when
     `dataOnly` (default true), else Label when `labelOnly`, else None — and `applyMatchedLines`
     has an arm for Label and an arm for Data and no third one.  Six of
     `033_Event_planning_tracker`'s eighty-four are this shape.
  1  a bare `<pivotArea outline="0"/>` naming a `dxf` that states `Liberation Mono` and 8 pt.
     **The whole data area**, because a format with no references matches every line through the
     broad path.  This is the arm that carries `033`.
  2  the same shape naming a `dxf` whose `<patternFill>` states a `bgColor` and **no**
     `patternType`.  **A solid fill of the background colour** — deliberately green where the
     cells' own hard fill is yellow, so that the two candidate sources are told apart rather
     than agreeing by accident — `Fill::finalizeImport`'s dxf arm
     moves the background into the pattern colour and forces the pattern solid.
  3  `<pivotArea grandRow="1"/>` with a bold `dxf`.  26.2.4.2 has no grand-total short circuit, so
     this is another all-lines Data record and lands on the data area as well; the fixture states
     it to pin that, because the C++ tree read here does have one.
  4  a Label record with a reference on the data dimension naming index 1.  **One column header
     cell**, the second of the two data fields.

    make-format-fixture.py <out.xlsx>
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
<sheet name="Formatted" sheetId="2" r:id="rId2"/>
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
#  One record per arm.  dxf 0 is inert by kind, 1 and 2 are all-lines Data records, 3 is the
#  grand-total arm that 26.2.4.2 does not short-circuit, and 4 is a label with a reference.
FORMATS = '''<formats count="5">
<format dxfId="0"><pivotArea type="all" dataOnly="0" outline="0" fieldPosition="0"/></format>
<format dxfId="1"><pivotArea outline="0" fieldPosition="0"/></format>
<format dxfId="2"><pivotArea outline="0" fieldPosition="0"/></format>
<format dxfId="3"><pivotArea grandRow="1" outline="0" fieldPosition="0"/></format>
<format dxfId="4"><pivotArea dataOnly="0" labelOnly="1" outline="0" fieldPosition="0"><references count="1"><reference field="4294967294" count="1"><x v="1"/></reference></references></pivotArea></format>
</formats>'''

#  dxf 0 states an 18 pt Liberation Serif that must reach nothing at all; 1 the face and size the
#  records put back; 2 the fill; 3 a weight; 4 a colour.
DXFS = '''<dxfs count="5">
<dxf><font><sz val="18"/><name val="Liberation Serif"/></font></dxf>
<dxf><font><sz val="8"/><name val="Liberation Mono"/><family val="3"/></font></dxf>
<dxf><fill><patternFill><bgColor rgb="FF00B050"/></patternFill></fill></dxf>
<dxf><font><b/></font></dxf>
<dxf><font><color rgb="FFFF0000"/></font></dxf>
</dxfs>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="3"><font><sz val="11"/><color rgb="FF000000"/><name val="Liberation Sans"/></font><font><sz val="18"/><color rgb="FFFF0000"/><name val="Liberation Serif"/></font><font><b/><sz val="8"/><color rgb="FF00A000"/><name val="Liberation Mono"/></font></fonts>
<fills count="3"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill><fill><patternFill patternType="solid"><fgColor rgb="FFFFFF00"/><bgColor indexed="64"/></patternFill></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
<cellXfs count="2"><xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1"/><xf numFmtId="0" fontId="2" fillId="2" borderId="0" xfId="0" applyFont="1" applyFill="1"/></cellXfs>
</styleSheet>'''
STYLES = STYLES.replace('</styleSheet>', DXFS + '</styleSheet>')

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
<pivotTableDefinition xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" name="PivotFormatted" cacheId="1" dataOnRows="0" applyNumberFormats="0" applyBorderFormats="0" applyFontFormats="0" applyPatternFormats="0" applyAlignmentFormats="0" applyWidthHeightFormats="1" dataCaption="Values" updatedVersion="6" minRefreshableVersion="3" useAutoFormatting="1" itemPrintTitles="1" createdVersion="4" indent="0" compact="0" compactData="0" outline="0" outlineData="0" multipleFieldFilters="0">
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
        z.writestr('xl/pivotTables/pivotTable1.xml',
                   CLEARED.replace('</pivotTableDefinition>', FORMATS + '</pivotTableDefinition>'))
        z.writestr('xl/pivotTables/_rels/pivotTable1.xml.rels', PIVOT_RELS)


main()
