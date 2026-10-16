#!/usr/bin/env python3
"""Builds the two chart fixtures this round's tests read.

Both hold one bar chart with *no* `c:spPr` on the value axis, the category axis or the
value axis' `c:majorGridlines`, so every one of those lines takes the automatic format —
whose width is the theme's first `a:lnStyleLst` entry.  The theme states `w="9525"`, which
the reference keeps as 26 hundredths of a millimetre and draws at 0.737 pt.

Usage: make-chart-fixtures.py <outdir>
"""
import sys, zipfile, os

THEME = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="Probe">
<a:themeElements>
<a:clrScheme name="Probe"><a:dk1><a:srgbClr val="000000"/></a:dk1><a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>
<a:dk2><a:srgbClr val="44546A"/></a:dk2><a:lt2><a:srgbClr val="E7E6E6"/></a:lt2>
<a:accent1><a:srgbClr val="4472C4"/></a:accent1><a:accent2><a:srgbClr val="ED7D31"/></a:accent2>
<a:accent3><a:srgbClr val="A5A5A5"/></a:accent3><a:accent4><a:srgbClr val="FFC000"/></a:accent4>
<a:accent5><a:srgbClr val="5B9BD5"/></a:accent5><a:accent6><a:srgbClr val="70AD47"/></a:accent6>
<a:hlink><a:srgbClr val="0563C1"/></a:hlink><a:folHlink><a:srgbClr val="954F72"/></a:folHlink></a:clrScheme>
<a:fontScheme name="Probe"><a:majorFont><a:latin typeface="Liberation Sans"/><a:ea typeface=""/><a:cs typeface=""/></a:majorFont>
<a:minorFont><a:latin typeface="Liberation Sans"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont></a:fontScheme>
<a:fmtScheme name="Probe">
<a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>
<a:lnStyleLst>
<a:ln w="9525" cap="flat" cmpd="sng" algn="ctr"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/></a:ln>
<a:ln w="25400" cap="flat" cmpd="sng" algn="ctr"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/></a:ln>
<a:ln w="38100" cap="flat" cmpd="sng" algn="ctr"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/></a:ln>
</a:lnStyleLst>
<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>
<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>
</a:fmtScheme></a:themeElements></a:theme>'''

# One bar series, one line series with a diamond marker that states its own outline, and a
# value axis whose gridlines and axis line state nothing at all.
CHART = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<c:chartSpace xmlns:c="http://schemas.openxmlformats.org/drawingml/2006/chart"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<c:chart><c:plotArea><c:layout/>
<c:barChart><c:barDir val="col"/><c:grouping val="clustered"/>
<c:ser><c:idx val="0"/><c:order val="0"/>
<c:spPr><a:solidFill><a:srgbClr val="4472C4"/></a:solidFill></c:spPr>
<c:cat><c:strRef><c:f>Sheet1!$A$1:$A$4</c:f><c:strCache><c:ptCount val="4"/>
<c:pt idx="0"><c:v>A</c:v></c:pt><c:pt idx="1"><c:v>B</c:v></c:pt>
<c:pt idx="2"><c:v>C</c:v></c:pt><c:pt idx="3"><c:v>D</c:v></c:pt></c:strCache></c:strRef></c:cat>
<c:val><c:numRef><c:f>Sheet1!$B$1:$B$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val="4"/>
<c:pt idx="0"><c:v>3</c:v></c:pt><c:pt idx="1"><c:v>5</c:v></c:pt>
<c:pt idx="2"><c:v>2</c:v></c:pt><c:pt idx="3"><c:v>6</c:v></c:pt></c:numCache></c:numRef></c:val>
<c:axId val="111111111"/><c:axId val="222222222"/></c:ser>
<c:axId val="111111111"/><c:axId val="222222222"/></c:barChart>
<c:lineChart><c:grouping val="standard"/>
<c:ser><c:idx val="1"/><c:order val="1"/>
<c:spPr><a:ln w="28575"><a:solidFill><a:srgbClr val="ED7D31"/></a:solidFill></a:ln></c:spPr>
<c:marker><c:symbol val="diamond"/><c:size val="7"/>
<c:spPr><a:solidFill><a:srgbClr val="ED7D31"/></a:solidFill>
<a:ln><a:solidFill><a:srgbClr val="203864"/></a:solidFill></a:ln></c:spPr></c:marker>
<c:val><c:numRef><c:f>Sheet1!$C$1:$C$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val="4"/>
<c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>4</c:v></c:pt>
<c:pt idx="2"><c:v>3</c:v></c:pt><c:pt idx="3"><c:v>5</c:v></c:pt></c:numCache></c:numRef></c:val>
<c:axId val="111111111"/><c:axId val="222222222"/></c:ser>
<c:marker val="1"/>
<c:axId val="111111111"/><c:axId val="222222222"/></c:lineChart>
<c:catAx><c:axId val="111111111"/><c:scaling><c:orientation val="minMax"/></c:scaling>
<c:delete val="0"/><c:axPos val="b"/><c:crossAx val="222222222"/></c:catAx>
<c:valAx><c:axId val="222222222"/><c:scaling><c:orientation val="minMax"/></c:scaling>
<c:delete val="0"/><c:axPos val="l"/><c:majorGridlines/><c:crossAx val="111111111"/></c:valAx>
</c:plotArea><c:plotVisOnly val="1"/></c:chart></c:chartSpace>'''

SHEET = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheetData>
<row r="1"><c r="A1" t="inlineStr"><is><t>A</t></is></c><c r="B1"><v>3</v></c><c r="C1"><v>1</v></c></row>
<row r="2"><c r="A2" t="inlineStr"><is><t>B</t></is></c><c r="B2"><v>5</v></c><c r="C2"><v>4</v></c></row>
<row r="3"><c r="A3" t="inlineStr"><is><t>C</t></is></c><c r="B3"><v>2</v></c><c r="C3"><v>3</v></c></row>
<row r="4"><c r="A4" t="inlineStr"><is><t>D</t></is></c><c r="B4"><v>6</v></c><c r="C4"><v>5</v></c></row>
</sheetData>
<drawing r:id="rId1"/></worksheet>'''

DRAWING = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:c="http://schemas.openxmlformats.org/drawingml/2006/chart"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<xdr:absoluteAnchor><xdr:pos x="381000" y="381000"/><xdr:ext cx="5486400" cy="3200400"/>
<xdr:graphicFrame><xdr:nvGraphicFramePr><xdr:cNvPr id="2" name="Chart 1"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr>
<xdr:xfrm><a:off x="381000" y="381000"/><a:ext cx="5486400" cy="3200400"/></xdr:xfrm>
<a:graphic><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/chart">
<c:chart r:id="rId1"/></a:graphicData></a:graphic></xdr:graphicFrame>
<xdr:clientData/></xdr:absoluteAnchor></xdr:wsDr>'''

DOC = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:c="http://schemas.openxmlformats.org/drawingml/2006/chart"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<w:body><w:p><w:r><w:drawing>
<wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="5486400" cy="3200400"/><wp:docPr id="1" name="Chart 1"/>
<a:graphic><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/chart">
<c:chart r:id="rId5"/></a:graphicData></a:graphic></wp:inline>
</w:drawing></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr></w:body></w:document>'''


# A third fixture: three line series that differ only in what they say about their own line.
#   idx 0  states `a:ln w="28575"`            -> 79 hundredths of a millimetre
#   idx 1  states `a:ln` with a fill, no `w`  -> 35, a constant
#   idx 2  states no `c:spPr` at all          -> the theme's 9525 times style 18's 500 % = 132
SERIES_CHART = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<c:chartSpace xmlns:c="http://schemas.openxmlformats.org/drawingml/2006/chart"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<c:chart><c:plotArea><c:layout/>
<c:barChart><c:barDir val="col"/><c:grouping val="clustered"/>
<c:ser><c:idx val="3"/><c:order val="3"/>
<c:spPr><a:solidFill><a:srgbClr val="4472C4"/></a:solidFill>
<a:ln><a:solidFill><a:srgbClr val="203864"/></a:solidFill></a:ln></c:spPr>
<c:val><c:numRef><c:f>Sheet1!$B$1:$B$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val="4"/>
<c:pt idx="0"><c:v>3</c:v></c:pt><c:pt idx="1"><c:v>5</c:v></c:pt>
<c:pt idx="2"><c:v>2</c:v></c:pt><c:pt idx="3"><c:v>6</c:v></c:pt></c:numCache></c:numRef></c:val>
<c:axId val="111111111"/><c:axId val="222222222"/></c:ser>
<c:axId val="111111111"/><c:axId val="222222222"/></c:barChart>
<c:lineChart><c:grouping val="standard"/>
<c:ser><c:idx val="0"/><c:order val="0"/>
<c:spPr><a:ln w="28575"><a:solidFill><a:srgbClr val="ED7D31"/></a:solidFill></a:ln></c:spPr>
<c:marker><c:symbol val="none"/></c:marker>
<c:val><c:numRef><c:f>Sheet1!$B$1:$B$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val="4"/>
<c:pt idx="0"><c:v>3</c:v></c:pt><c:pt idx="1"><c:v>5</c:v></c:pt>
<c:pt idx="2"><c:v>2</c:v></c:pt><c:pt idx="3"><c:v>6</c:v></c:pt></c:numCache></c:numRef></c:val>
<c:axId val="111111111"/><c:axId val="222222222"/></c:ser>
<c:ser><c:idx val="1"/><c:order val="1"/>
<c:spPr><a:ln><a:solidFill><a:srgbClr val="70AD47"/></a:solidFill></a:ln></c:spPr>
<c:marker><c:symbol val="none"/></c:marker>
<c:val><c:numRef><c:f>Sheet1!$C$1:$C$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val="4"/>
<c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>4</c:v></c:pt>
<c:pt idx="2"><c:v>3</c:v></c:pt><c:pt idx="3"><c:v>5</c:v></c:pt></c:numCache></c:numRef></c:val>
<c:axId val="111111111"/><c:axId val="222222222"/></c:ser>
<c:ser><c:idx val="2"/><c:order val="2"/>
<c:marker><c:symbol val="none"/></c:marker>
<c:val><c:numRef><c:f>Sheet1!$D$1:$D$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val="4"/>
<c:pt idx="0"><c:v>6</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
<c:pt idx="2"><c:v>5</c:v></c:pt><c:pt idx="3"><c:v>1</c:v></c:pt></c:numCache></c:numRef></c:val>
<c:axId val="111111111"/><c:axId val="222222222"/></c:ser>
<c:marker val="1"/>
<c:axId val="111111111"/><c:axId val="222222222"/></c:lineChart>
<c:catAx><c:axId val="111111111"/><c:scaling><c:orientation val="minMax"/></c:scaling>
<c:delete val="0"/><c:axPos val="b"/><c:crossAx val="222222222"/></c:catAx>
<c:valAx><c:axId val="222222222"/><c:scaling><c:orientation val="minMax"/></c:scaling>
<c:delete val="0"/><c:axPos val="l"/><c:crossAx val="111111111"/></c:valAx>
</c:plotArea><c:plotVisOnly val="1"/></c:chart><c:style val="18"/></c:chartSpace>'''

SERIES_SHEET = SHEET.replace(
    '<c r="C1"><v>1</v></c>', '<c r="C1"><v>1</v></c><c r="D1"><v>6</v></c>').replace(
    '<c r="C2"><v>4</v></c>', '<c r="C2"><v>4</v></c><c r="D2"><v>2</v></c>').replace(
    '<c r="C3"><v>3</v></c>', '<c r="C3"><v>3</v></c><c r="D3"><v>5</v></c>').replace(
    '<c r="C4"><v>5</v></c>', '<c r="C4"><v>5</v></c><c r="D4"><v>1</v></c>')


def xlsx(path, chart=None, sheet=None):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
                   '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
                   '<Default Extension="xml" ContentType="application/xml"/>'
                   '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
                   '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
                   '<Override PartName="/xl/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>'
                   '<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>'
                   '<Override PartName="/xl/charts/chart1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawingml.chart+xml"/>'
                   '</Types>')
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>'
                   '</Relationships>')
        z.writestr('xl/workbook.xml',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"'
                   ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
                   '<sheets><sheet name="Sheet1" sheetId="1" r:id="rId1"/></sheets></workbook>')
        z.writestr('xl/_rels/workbook.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>'
                   '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/>'
                   '</Relationships>')
        z.writestr('xl/worksheets/sheet1.xml', sheet or SHEET)
        z.writestr('xl/worksheets/_rels/sheet1.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing" Target="../drawings/drawing1.xml"/>'
                   '</Relationships>')
        z.writestr('xl/drawings/drawing1.xml', DRAWING)
        z.writestr('xl/drawings/_rels/drawing1.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart" Target="../charts/chart1.xml"/>'
                   '</Relationships>')
        z.writestr('xl/charts/chart1.xml', chart or CHART)
        z.writestr('xl/theme/theme1.xml', THEME)
        # A worksheet with no <sheetPr> and no pageSetup prints at 100 per cent.


def docx(path):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
                   '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
                   '<Default Extension="xml" ContentType="application/xml"/>'
                   '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
                   '<Override PartName="/word/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>'
                   '<Override PartName="/word/charts/chart1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawingml.chart+xml"/>'
                   '</Types>')
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
                   '</Relationships>')
        z.writestr('word/document.xml', DOC)
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId4" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/>'
                   '<Relationship Id="rId5" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart" Target="charts/chart1.xml"/>'
                   '</Relationships>')
        z.writestr('word/charts/chart1.xml', CHART)
        z.writestr('word/theme/theme1.xml', THEME)


if __name__ == '__main__':
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    xlsx(os.path.join(out, 'sheet-chart-auto-line.xlsx'))
    xlsx(os.path.join(out, 'sheet-chart-series-width.xlsx'), SERIES_CHART, SERIES_SHEET)
    docx(os.path.join(out, 'words-chart-auto-line.docx'))
    print('wrote three fixtures into', out)
