#!/usr/bin/env python3
"""Author the smallest workbook that exercises the generated pivot grid.

Two row fields and two data fields, with a subtotal under each outer member and a grand total —
the four shapes `ScDPOutput` frames differently: a member's block, a nested member's own column,
a subtotal's strip across the row headers, and the table's outer edge. Two data fields rather
than one so the pivot keeps a column field: with one, the bare data-layout placeholder in
`colFields` leaves Calc with no column field at all (`dpoutput.cxx`:1205) and Excel's layout and
Calc's differ by a row.

Written by hand rather than by a spreadsheet application so that every number the test asserts
can be traced to one element of the file. `styles.xml` states a single border and it is empty
and a single `cellXf` with no weight, alignment or indent, so nothing the test expects is
reachable from the file. Byte-for-byte reproducible: re-running this over
`tests/corpus/features/sheet-pivot-grid.xlsx` produces the same parts with the same contents.

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
<sheets><sheet name="Data" sheetId="1" r:id="rId1"/><sheet name="Pivot" sheetId="2" r:id="rId2"/></sheets>
<pivotCaches><pivotCache cacheId="1" r:id="rId4"/></pivotCaches>
</workbook>'''

WB_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet2.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId4" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition" Target="pivotCache/pivotCacheDefinition1.xml"/>
</Relationships>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="2"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>
</styleSheet>'''

DATA_ROWS = [
    ('Region', 'Product', 'Amount'),
    ('North', 'Apple', 10), ('North', 'Pear', 20),
    ('South', 'Apple', 30), ('South', 'Pear', 40),
]

def cell(ref, value):
    if isinstance(value, (int, float)):
        return '<c r="%s"><v>%s</v></c>' % (ref, value)
    return '<c r="%s" t="str"><v>%s</v></c>' % (ref, value)

def sheet(rows):
    out = []
    for r, values in enumerate(rows, 1):
        cs = ''.join(cell('%s%d' % (chr(65 + c), r), v) for c, v in enumerate(values) if v != '')
        out.append('<row r="%d">%s</row>' % (r, cs))
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
            '<sheetData>%s</sheetData></worksheet>' % ''.join(out))

# The laid-out pivot exactly as Excel writes it, and exactly as the location below describes it.
PIVOT_ROWS = [
    ('', '', 'Values', ''),
    ('Region', 'Product', 'Sum of Amount', 'Count of Amount'),
    ('North', 'Apple', 10, 1),
    ('', 'Pear', 20, 1),
    ('North Total', '', 30, 2),
    ('South', 'Apple', 30, 1),
    ('', 'Pear', 40, 1),
    ('South Total', '', 70, 2),
    ('Grand Total', '', 100, 4),
]

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

PIVOT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<pivotTableDefinition xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" name="PivotTable1" cacheId="1" dataOnRows="0" applyNumberFormats="0" applyBorderFormats="0" applyFontFormats="0" applyPatternFormats="0" applyAlignmentFormats="0" applyWidthHeightFormats="1" dataCaption="Values" updatedVersion="6" minRefreshableVersion="3" useAutoFormatting="1" itemPrintTitles="1" createdVersion="4" indent="0" compact="0" compactData="0" outline="0" outlineData="0" gridDropZones="1" multipleFieldFilters="0">
<location ref="A1:D9" firstHeaderRow="1" firstDataRow="2" firstDataCol="2"/>
<pivotFields count="3">
<pivotField axis="axisRow" compact="0" outline="0" subtotalTop="0" showAll="0"><items count="3"><item x="0"/><item x="1"/><item t="default"/></items></pivotField>
<pivotField axis="axisRow" compact="0" outline="0" subtotalTop="0" showAll="0" defaultSubtotal="0"><items count="2"><item x="0"/><item x="1"/></items></pivotField>
<pivotField dataField="1" compact="0" outline="0" showAll="0"/>
</pivotFields>
<rowFields count="2"><field x="0"/><field x="1"/></rowFields>
<rowItems count="7">
<i><x v="0"/><x v="0"/></i>
<i r="1"><x v="1"/></i>
<i t="default"><x v="0"/></i>
<i><x v="1"/><x v="0"/></i>
<i r="1"><x v="1"/></i>
<i t="default"><x v="1"/></i>
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

SHEET2_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
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
        z.writestr('xl/worksheets/sheet2.xml', sheet(PIVOT_ROWS))
        z.writestr('xl/worksheets/_rels/sheet2.xml.rels', SHEET2_RELS)
        z.writestr('xl/pivotCache/pivotCacheDefinition1.xml', CACHE_DEF)
        z.writestr('xl/pivotCache/_rels/pivotCacheDefinition1.xml.rels', CACHE_DEF_RELS)
        z.writestr('xl/pivotCache/pivotCacheRecords1.xml', CACHE_RECORDS)
        z.writestr('xl/pivotTables/pivotTable1.xml', PIVOT)
        z.writestr('xl/pivotTables/_rels/pivotTable1.xml.rels', PIVOT_RELS)

main()
