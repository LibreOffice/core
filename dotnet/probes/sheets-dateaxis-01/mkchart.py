#!/usr/bin/env python3
"""Build a minimal .xlsx holding one line chart on a c:dateAx, for measuring
LibreOffice's automatic date-axis tick rule."""
import datetime, os, shutil, sys, zipfile

NULL = datetime.date(1899, 12, 30)


def serial(d):
    if d is None:
        return None
    if isinstance(d, (int, float)):
        return d
    return (d - NULL).days


CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>
<Override PartName="/xl/charts/chart1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawingml.chart+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

WB = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="Data" sheetId="1" r:id="rId1"/></sheets></workbook>'''

WBRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<numFmts count="1"><numFmt numFmtId="164" formatCode="DD/MM/YY"/></numFmts>
<fonts count="1"><font><sz val="10"/><name val="Arial"/></font></fonts>
<fills count="2"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="2"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>
<xf numFmtId="164" fontId="0" fillId="0" borderId="0" xfId="0" applyNumberFormat="1"/></cellXfs>
</styleSheet>'''

SHEETRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing" Target="../drawings/drawing1.xml"/>
</Relationships>'''

DRAWRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart" Target="../charts/chart1.xml"/>
</Relationships>'''


def drawing(cx_emu, cy_emu):
    return f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing" xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<xdr:absoluteAnchor><xdr:pos x="0" y="0"/><xdr:ext cx="{cx_emu}" cy="{cy_emu}"/>
<xdr:graphicFrame><xdr:nvGraphicFramePr><xdr:cNvPr id="2" name="Chart 1"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr>
<xdr:xfrm><a:off x="0" y="0"/><a:ext cx="{cx_emu}" cy="{cy_emu}"/></xdr:xfrm>
<a:graphic><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/chart">
<c:chart xmlns:c="http://schemas.openxmlformats.org/drawingml/2006/chart" r:id="rId1"/>
</a:graphicData></a:graphic></xdr:graphicFrame><xdr:clientData/></xdr:absoluteAnchor></xdr:wsDr>'''


def sheet(dates, values):
    rows = []
    for i, (d, v) in enumerate(zip(dates, values), start=2):
        cells = ''
        if serial(d) is not None:
            cells += f'<c r="A{i}" s="1"><v>{serial(d)}</v></c>'
        if v is not None:
            cells += f'<c r="B{i}"><v>{v}</v></c>'
        rows.append(f'<row r="{i}">{cells}</row>')
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
            'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
            '<sheetData>' + ''.join(rows) + '</sheetData>'
            '<drawing r:id="rId1"/></worksheet>')


def chart(dates, values, scaling='', fmt='DD/MM/YYYY', timeunit='', blanks='', axkind='dateAx', kind='line'):
    n = len(dates)
    cat = ''.join(f'<c:pt idx="{i}"><c:v>{serial(d)}</c:v></c:pt>'
                  for i, d in enumerate(dates) if serial(d) is not None)
    val = ''.join(f'<c:pt idx="{i}"><c:v>{v}</c:v></c:pt>'
                  for i, v in enumerate(values) if v is not None)
    return f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<c:chartSpace xmlns:c="http://schemas.openxmlformats.org/drawingml/2006/chart" xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<c:chart><c:autoTitleDeleted val="1"/><c:plotArea><c:layout/>
<c:{kind}Chart><c:grouping val="standard"/><c:varyColors val="0"/>
<c:ser><c:idx val="0"/><c:order val="0"/><c:marker><c:symbol val="none"/></c:marker>
<c:cat><c:numRef><c:f>Data!$A$2:$A${n + 1}</c:f><c:numCache><c:formatCode>{fmt}</c:formatCode><c:ptCount val="{n}"/>{cat}</c:numCache></c:numRef></c:cat>
<c:val><c:numRef><c:f>Data!$B$2:$B${n + 1}</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val="{n}"/>{val}</c:numCache></c:numRef></c:val>
</c:ser><c:axId val="111"/><c:axId val="222"/></c:{kind}Chart>
<c:{axkind}><c:axId val="111"/><c:scaling><c:orientation val="minMax"/>{scaling}</c:scaling><c:delete val="0"/><c:axPos val="b"/>
<c:numFmt formatCode="{fmt}" sourceLinked="0"/><c:majorTickMark val="out"/><c:minorTickMark val="none"/><c:tickLblPos val="nextTo"/>
<c:txPr><a:bodyPr rot="0"/><a:lstStyle/><a:p><a:pPr><a:defRPr sz="700"><a:latin typeface="Arial"/></a:defRPr></a:pPr></a:p></c:txPr>
<c:crossAx val="222"/><c:crosses val="autoZero"/><c:auto val="1"/><c:lblOffset val="100"/>{timeunit}</c:{axkind}>
<c:valAx><c:axId val="222"/><c:scaling><c:orientation val="minMax"/></c:scaling><c:delete val="0"/><c:axPos val="l"/>
<c:numFmt formatCode="General" sourceLinked="0"/><c:majorTickMark val="out"/><c:minorTickMark val="none"/><c:tickLblPos val="nextTo"/>
<c:txPr><a:bodyPr/><a:lstStyle/><a:p><a:pPr><a:defRPr sz="700"><a:latin typeface="Arial"/></a:defRPr></a:pPr></a:p></c:txPr>
<c:crossAx val="111"/><c:crosses val="autoZero"/><c:crossBetween val="between"/></c:valAx>
</c:plotArea><c:plotVisOnly val="1"/>{blanks}</c:chart></c:chartSpace>'''


def build(path, dates, values, cx_cm=24.0, cy_cm=12.0, scaling='', fmt='DD/MM/YYYY', timeunit='', blanks='', axkind='dateAx', kind='line'):
    emu = lambda cm: int(cm * 360000)
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('xl/workbook.xml', WB)
        z.writestr('xl/_rels/workbook.xml.rels', WBRELS)
        z.writestr('xl/styles.xml', STYLES)
        z.writestr('xl/worksheets/sheet1.xml', sheet(dates, values))
        z.writestr('xl/worksheets/_rels/sheet1.xml.rels', SHEETRELS)
        z.writestr('xl/drawings/drawing1.xml', drawing(emu(cx_cm), emu(cy_cm)))
        z.writestr('xl/drawings/_rels/drawing1.xml.rels', DRAWRELS)
        z.writestr('xl/charts/chart1.xml', chart(dates, values, scaling, fmt, timeunit, blanks, axkind, kind))
