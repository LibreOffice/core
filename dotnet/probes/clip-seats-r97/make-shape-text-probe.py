#!/usr/bin/env python3
"""Author a minimal .xlsx whose one filled rectangle *carrying text* straddles a
page-column break, so the reference can be asked the narrower question O12 asks: when the
drawing-layer clip cuts a shape, does the shape's own text go with it?

The text is right-aligned inside the shape, so on the first column band it lies wholly
outside the block the drawing layer is clipped to.  Whether 26.2.4.2 still writes those
glyphs into that page's text layer is the whole answer.

Usage: make-shape-text-probe.py OUT.xlsx [--cols N] [--rows N] [--from-col C --to-col C]
"""
import sys, zipfile, argparse

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

WB = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="Probe" sheetId="1" r:id="rId1"/></sheets>
</workbook>'''

WBR = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>'''

STY = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>
</styleSheet>'''

SHR = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing" Target="../drawings/drawing1.xml"/>
</Relationships>'''


def col_name(i):
    s = ''
    i += 1
    while i:
        i, r = divmod(i - 1, 26)
        s = chr(65 + r) + s
    return s


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('out')
    ap.add_argument('--cols', type=int, default=16)
    ap.add_argument('--rows', type=int, default=80)
    ap.add_argument('--from-col', type=int, default=1)
    ap.add_argument('--from-row', type=int, default=1)
    ap.add_argument('--to-col', type=int, default=14)
    ap.add_argument('--to-row', type=int, default=70)
    ap.add_argument('--to-col-off', type=int, default=0, help='EMU offset into --to-col')
    ap.add_argument('--right-word', default='RIGHTMARGINWORD')
    ap.add_argument('--left-word', default='LEFTMARGINWORD')
    a = ap.parse_args()

    rows = []
    for r in range(1, a.rows + 1):
        cells = ''.join(
            f'<c r="{col_name(c)}{r}" t="inlineStr"><is><t>{col_name(c)}{r}</t></is></c>'
            for c in range(a.cols))
        rows.append(f'<row r="{r}">{cells}</row>')

    sheet = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
        'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
        f'<cols><col min="1" max="{a.cols}" width="14" customWidth="1"/></cols>'
        f'<sheetData>{"".join(rows)}</sheetData>'
        '<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75" header="0.3" footer="0.3"/>'
        '<pageSetup paperSize="9" orientation="portrait"/>'
        '<drawing r:id="rId1"/></worksheet>')

    drawing = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing" '
        'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
        '<xdr:twoCellAnchor editAs="oneCell">'
        f'<xdr:from><xdr:col>{a.from_col}</xdr:col><xdr:colOff>0</xdr:colOff>'
        f'<xdr:row>{a.from_row}</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>'
        f'<xdr:to><xdr:col>{a.to_col}</xdr:col><xdr:colOff>{a.to_col_off}</xdr:colOff>'
        f'<xdr:row>{a.to_row}</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>'
        '<xdr:sp macro="" textlink=""><xdr:nvSpPr>'
        '<xdr:cNvPr id="2" name="Rectangle 1"/><xdr:cNvSpPr/></xdr:nvSpPr>'
        '<xdr:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/></a:xfrm>'
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
        '<a:solidFill><a:srgbClr val="C00000"/></a:solidFill>'
        '<a:ln w="28575"><a:solidFill><a:srgbClr val="000000"/></a:solidFill></a:ln>'
        '</xdr:spPr><xdr:style/>'
        '<xdr:txBody><a:bodyPr wrap="square" anchor="t"/><a:lstStyle/>'
        f'<a:p><a:pPr algn="r"/><a:r><a:rPr lang="en-US" sz="1400"/>'
        f'<a:t>{a.right_word}</a:t></a:r></a:p>'
        f'<a:p><a:pPr algn="l"/><a:r><a:rPr lang="en-US" sz="1400"/>'
        f'<a:t>{a.left_word}</a:t></a:r></a:p>'
        '</xdr:txBody></xdr:sp><xdr:clientData/></xdr:twoCellAnchor>'
        '</xdr:wsDr>')

    with zipfile.ZipFile(a.out, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('xl/workbook.xml', WB)
        z.writestr('xl/_rels/workbook.xml.rels', WBR)
        z.writestr('xl/styles.xml', STY)
        z.writestr('xl/worksheets/sheet1.xml', sheet)
        z.writestr('xl/worksheets/_rels/sheet1.xml.rels', SHR)
        z.writestr('xl/drawings/drawing1.xml', drawing)
    print(a.out)


main()
