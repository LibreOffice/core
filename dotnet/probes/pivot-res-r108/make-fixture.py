#!/usr/bin/env python3
"""Author the smallest workbook that exercises the two layouts r107 declined.

Two pivots over one cache, on two sheets of their own:

`Hidden`  states `firstHeaderRow="0"`, which the OOXML import turns into
          `ScDPObject::SetHideHeader(true)` (`sc/source/filter/oox/pivottablebuffer.cxx`:1368)
          and `CalcSizes` into `mnHeaderSize = 0` (`dpoutput.cxx`:884) — the field-button row
          is gone, so the column members sit on the table's own first row and take its outer
          rule.

`Cleared` is `Hidden` again over cells that hard-state a centring, an indent and a bold font,
          none of which survives `clearContents(… HARDATTR | STYLES …)`
          (`pivottablebuffer.cxx`:1331-1336) — while the workbook's *default* `cellXf`, which
          states a right justification, does, because clearing takes a cell back to the Default
          cell style rather than to nothing.

`Packed`  states two row fields, both compact, which `GetColumnsForRowFields` packs into one
          row-label column (`dpoutput.cxx`:854-868). The button cell then takes
          `MultiFieldCell` rather than `FieldCell` and is not boxed, and the outer field's
          indent step falls on the inner field's members in the same column.

Written by hand rather than by a spreadsheet application so that every number the test asserts
can be traced to one element of the file. `styles.xml` states one border and it is empty, so no
edge the test expects is reachable from the file, and two `cellXf` — the default stating a right
justification the reference keeps, and one stating a centring, an indent and a bold weight it
throws away. Reproducible part for part: re-running it over
`tests/corpus/features/sheet-pivot-packed.xlsx` writes the same parts with the same contents,
though not the same zip bytes — a zip entry stores its own mtime. The kept justification is
written into `cellStyleXfs[0]` as well as into
`cellXfs[0]`, because it is the `Normal` cell style that becomes Calc's Default and survives the
clear; every producer writes the two the same, and this fixture does too rather than testing a
distinction no corpus workbook makes.

    make-fixture.py <out.xlsx>
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
<Override PartName="/xl/worksheets/sheet4.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/pivotCache/pivotCacheDefinition1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheDefinition+xml"/>
<Override PartName="/xl/pivotCache/pivotCacheRecords1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheRecords+xml"/>
<Override PartName="/xl/pivotTables/pivotTable1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"/>
<Override PartName="/xl/pivotTables/pivotTable2.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"/>
<Override PartName="/xl/pivotTables/pivotTable3.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
</Types>'''

ROOT_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

WORKBOOK = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="Data" sheetId="1" r:id="rId1"/><sheet name="Hidden" sheetId="2" r:id="rId2"/><sheet name="Packed" sheetId="3" r:id="rId3"/><sheet name="Cleared" sheetId="4" r:id="rId6"/></sheets>
<pivotCaches><pivotCache cacheId="1" r:id="rId5"/></pivotCaches>
</workbook>'''

WB_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet2.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet3.xml"/>
<Relationship Id="rId4" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId5" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition" Target="pivotCache/pivotCacheDefinition1.xml"/>
<Relationship Id="rId6" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet4.xml"/>
</Relationships>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="2"><font><sz val="11"/><name val="Calibri"/></font><font><b/><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="2"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"><alignment horizontal="right"/></xf></cellStyleXfs>
<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
<cellXfs count="2"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyAlignment="1"><alignment horizontal="right"/></xf><xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1" applyAlignment="1"><alignment horizontal="center" indent="3"/></xf></cellXfs>
</styleSheet>'''

DATA_ROWS = [
    ('Region', 'Product', 'Amount'),
    ('North', 'Apple', 10), ('North', 'Pear', 20),
    ('South', 'Apple', 30), ('South', 'Pear', 40),
]

#  The laid-out pivots exactly as Excel writes them, and exactly as the locations below
#  describe them.  A hidden header has no field-button row at all: the column members are the
#  table's first row.
HIDDEN_ROWS = [
    ('Region', 'Sum of Amount', 'Count of Amount'),
    ('North', 30, 2),
    ('South', 70, 2),
    ('Grand Total', 100, 4),
]

#  Two compact row fields share one column: the outer member's own row carries its subtotal,
#  and the inner members follow underneath it in the same column.
PACKED_ROWS = [
    ('', 'Values', ''),
    ('Row Labels', 'Sum of Amount', 'Count of Amount'),
    ('North', 30, 2),
    ('Apple', 10, 1),
    ('Pear', 20, 1),
    ('South', 70, 2),
    ('Apple', 30, 1),
    ('Pear', 40, 1),
    ('Grand Total', 100, 4),
]


def cell(ref, value, style=0):
    s = ' s="%d"' % style if style else ''
    if isinstance(value, (int, float)):
        return '<c r="%s"%s><v>%s</v></c>' % (ref, s, value)
    return '<c r="%s"%s t="str"><v>%s</v></c>' % (ref, s, value)


def sheet(rows, style=0):
    out = []
    for r, values in enumerate(rows, 1):
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

#  One row field, laid out flat, and no field-button row: firstHeaderRow="0".
HIDDEN = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<pivotTableDefinition xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" name="PivotHidden" cacheId="1" dataOnRows="0" applyNumberFormats="0" applyBorderFormats="0" applyFontFormats="0" applyPatternFormats="0" applyAlignmentFormats="0" applyWidthHeightFormats="1" dataCaption="Values" updatedVersion="6" minRefreshableVersion="3" useAutoFormatting="1" itemPrintTitles="1" createdVersion="4" indent="0" compact="0" compactData="0" outline="0" outlineData="0" multipleFieldFilters="0">
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

#  Two row fields, both compact — none of subtotalTop, outline or compact is stated and all
#  three default to true — so Calc packs them into the one column Excel also wrote.
PACKED = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<pivotTableDefinition xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" name="PivotPacked" cacheId="1" dataOnRows="0" applyNumberFormats="0" applyBorderFormats="0" applyFontFormats="0" applyPatternFormats="0" applyAlignmentFormats="0" applyWidthHeightFormats="1" dataCaption="Values" updatedVersion="6" minRefreshableVersion="3" useAutoFormatting="1" itemPrintTitles="1" createdVersion="4" indent="0" outline="1" outlineData="1" compact="1" compactData="1" multipleFieldFilters="0">
<location ref="A1:C9" firstHeaderRow="1" firstDataRow="2" firstDataCol="1"/>
<pivotFields count="3">
<pivotField axis="axisRow" showAll="0"><items count="3"><item x="0"/><item x="1"/><item t="default"/></items></pivotField>
<pivotField axis="axisRow" showAll="0" defaultSubtotal="0"><items count="2"><item x="0"/><item x="1"/></items></pivotField>
<pivotField dataField="1" showAll="0"/>
</pivotFields>
<rowFields count="2"><field x="0"/><field x="1"/></rowFields>
<rowItems count="7">
<i><x v="0"/></i>
<i r="1"><x v="0"/></i>
<i r="1"><x v="1"/></i>
<i><x v="1"/></i>
<i r="1"><x v="0"/></i>
<i r="1"><x v="1"/></i>
<i t="grand"><x/></i>
</rowItems>
<colFields count="1"><field x="-2"/></colFields>
<colItems count="2"><i><x/></i><i i="1"><x v="1"/></i></colItems>
<dataFields count="2"><dataField name="Sum of Amount" fld="2" baseField="0" baseItem="0"/><dataField name="Count of Amount" fld="2" subtotal="count" baseField="0" baseItem="0"/></dataFields>
</pivotTableDefinition>'''

#  The same shape as Hidden, on a sheet whose every cell states cellXf 1 — a hard centring,
#  a hard indent and a bold font, all of which the reference clears before it draws.
CLEARED = HIDDEN.replace('name="PivotHidden"', 'name="PivotCleared"')

PIVOT_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition" Target="../pivotCache/pivotCacheDefinition1.xml"/>
</Relationships>'''


def sheet_rels(part):
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
            'relationships/pivotTable" Target="../pivotTables/%s"/>'
            '</Relationships>' % part)


def main():
    with zipfile.ZipFile(sys.argv[1], 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('xl/workbook.xml', WORKBOOK)
        z.writestr('xl/_rels/workbook.xml.rels', WB_RELS)
        z.writestr('xl/styles.xml', STYLES)
        z.writestr('xl/worksheets/sheet1.xml', sheet(DATA_ROWS))
        z.writestr('xl/worksheets/sheet2.xml', sheet(HIDDEN_ROWS))
        z.writestr('xl/worksheets/sheet3.xml', sheet(PACKED_ROWS))
        z.writestr('xl/worksheets/sheet4.xml', sheet(HIDDEN_ROWS, style=1))
        z.writestr('xl/worksheets/_rels/sheet2.xml.rels', sheet_rels('pivotTable1.xml'))
        z.writestr('xl/worksheets/_rels/sheet3.xml.rels', sheet_rels('pivotTable2.xml'))
        z.writestr('xl/worksheets/_rels/sheet4.xml.rels', sheet_rels('pivotTable3.xml'))
        z.writestr('xl/pivotCache/pivotCacheDefinition1.xml', CACHE_DEF)
        z.writestr('xl/pivotCache/_rels/pivotCacheDefinition1.xml.rels', CACHE_DEF_RELS)
        z.writestr('xl/pivotCache/pivotCacheRecords1.xml', CACHE_RECORDS)
        z.writestr('xl/pivotTables/pivotTable1.xml', HIDDEN)
        z.writestr('xl/pivotTables/_rels/pivotTable1.xml.rels', PIVOT_RELS)
        z.writestr('xl/pivotTables/pivotTable2.xml', PACKED)
        z.writestr('xl/pivotTables/_rels/pivotTable2.xml.rels', PIVOT_RELS)
        z.writestr('xl/pivotTables/pivotTable3.xml', CLEARED)
        z.writestr('xl/pivotTables/_rels/pivotTable3.xml.rels', PIVOT_RELS)


main()
