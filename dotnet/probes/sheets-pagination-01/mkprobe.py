#!/usr/bin/env python3
"""Build minimal one-cell XLSX probes varying only <pageSetup>, to measure what the
installed LibreOffice does with each paperSize/orientation combination."""
import sys, os, zipfile

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

WBRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>'''

WB = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="S1" sheetId="1" r:id="rId1"/></sheets>
</workbook>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>
</styleSheet>'''

SHEET = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="A1"/>
<sheetData><row r="1"><c r="A1" t="inlineStr"><is><t>PROBE</t></is></c></row></sheetData>
%s
</worksheet>'''


def build(path, pagesetup_xml):
    sheet = SHEET % pagesetup_xml
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('xl/workbook.xml', WB)
        z.writestr('xl/_rels/workbook.xml.rels', WBRELS)
        z.writestr('xl/styles.xml', STYLES)
        z.writestr('xl/worksheets/sheet1.xml', sheet)


CASES = {
    'none':            '',
    'ps121_land':      '<pageSetup paperSize="121" orientation="landscape"/>',
    'ps121_port':      '<pageSetup paperSize="121" orientation="portrait"/>',
    'ps121_noorient':  '<pageSetup paperSize="121"/>',
    'ps9_land':        '<pageSetup paperSize="9" orientation="landscape"/>',
    'ps9_port':        '<pageSetup paperSize="9" orientation="portrait"/>',
    'ps8_land':        '<pageSetup paperSize="8" orientation="landscape"/>',
    'ps8_port':        '<pageSetup paperSize="8" orientation="portrait"/>',
    'noPS_land':       '<pageSetup orientation="landscape"/>',
    'noPS_port':       '<pageSetup orientation="portrait"/>',
    'ps0_land':        '<pageSetup paperSize="0" orientation="landscape"/>',
    'ps119_land':      '<pageSetup paperSize="119" orientation="landscape"/>',
    'ps118_land':      '<pageSetup paperSize="118" orientation="landscape"/>',
    'ps117_land':      '<pageSetup paperSize="117" orientation="landscape"/>',
    'ps70_land':       '<pageSetup paperSize="70" orientation="landscape"/>',
    'ps256_land':      '<pageSetup paperSize="256" orientation="landscape"/>',
    'ps121_land_dflt': '<pageSetup paperSize="121" orientation="default"/>',
}

if __name__ == '__main__':
    outdir = sys.argv[1]
    os.makedirs(outdir, exist_ok=True)
    for name, ps in CASES.items():
        build(os.path.join(outdir, f'probe_{name}.xlsx'), ps)
    print(f'wrote {len(CASES)} probes to {outdir}')
