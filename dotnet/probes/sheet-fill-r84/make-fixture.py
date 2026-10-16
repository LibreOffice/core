#!/usr/bin/env python3
"""Build `sheet-shape-ink.xlsx`, the fixture the worksheet-ink tests are established on.

    make-fixture.py <outdir>

Five shapes on one sheet, each isolating one arm of the rule this round establishes:

  1  a `star5` stating its own `a:solidFill` and an `a:ln/a:solidFill` of a different colour
     and width, and **no text at all** — the case that was drawn nowhere;
  2  a `roundRect` stating no fill and no line and an `xdr:style` naming `a:fillRef idx="1"`
     and `a:lnRef idx="2"` over `accent1` — the theme format matrix, which 86 of the corpus's
     644 worksheet shapes really depend on;
  3  a `rect` stating `a:noFill` and `a:ln/a:noFill` under the same `xdr:style` — the control
     that must stay unpainted, because `a:noFill` beats the matrix;
  4  a `bentConnector3` turned a quarter, which is what
     `sc/source/filter/oox/drawingfragment.cxx`:299-330 reflects the anchor for;
  5  a group of two `ellipse`s, one stating its own fill and one stating `a:grpFill`, which
     takes the group's.

The theme is written out in full because the whole point of shapes 2 and 3 is what it holds:
`a:fillStyleLst` entry 1 is a flat `phClr` and entry 2 a gradient, and `a:lnStyleLst` entry 2
is 25400 EMU wide — none of which appears in the sheet's own markup.
"""
import os
import sys
import zipfile

CONTENT_TYPES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
<Override PartName="/xl/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>
<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>
</Types>'''

ROOT_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

WORKBOOK = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="Probe" sheetId="1" r:id="rId1"/></sheets>
</workbook>'''

WORKBOOK_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/>
</Relationships>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="10"/><name val="Liberation Sans"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf/></cellStyleXfs>
<cellXfs count="1"><xf xfId="0"/></cellXfs>
</styleSheet>'''

SHEET = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheetData>
<row r="1"><c r="A1" t="inlineStr"><is><t>one</t></is></c></row>
<row r="2"><c r="A2" t="inlineStr"><is><t>two</t></is></c></row>
</sheetData>
<drawing r:id="rId1"/>
</worksheet>'''

SHEET_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing" Target="../drawings/drawing1.xml"/>
</Relationships>'''

THEME = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="Probe">
<a:themeElements>
<a:clrScheme name="Probe">
<a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>
<a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1>
<a:dk2><a:srgbClr val="44546A"/></a:dk2><a:lt2><a:srgbClr val="E7E6E6"/></a:lt2>
<a:accent1><a:srgbClr val="4472C4"/></a:accent1><a:accent2><a:srgbClr val="ED7D31"/></a:accent2>
<a:accent3><a:srgbClr val="A5A5A5"/></a:accent3><a:accent4><a:srgbClr val="FFC000"/></a:accent4>
<a:accent5><a:srgbClr val="5B9BD5"/></a:accent5><a:accent6><a:srgbClr val="70AD47"/></a:accent6>
<a:hlink><a:srgbClr val="0563C1"/></a:hlink><a:folHlink><a:srgbClr val="954F72"/></a:folHlink>
</a:clrScheme>
<a:fontScheme name="Probe">
<a:majorFont><a:latin typeface="Liberation Sans"/><a:ea typeface=""/><a:cs typeface=""/></a:majorFont>
<a:minorFont><a:latin typeface="Liberation Sans"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont>
</a:fontScheme>
<a:fmtScheme name="Probe">
<a:fillStyleLst>
<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
<a:gradFill rotWithShape="1"><a:gsLst>
<a:gs pos="0"><a:schemeClr val="phClr"><a:tint val="60000"/></a:schemeClr></a:gs>
<a:gs pos="100000"><a:schemeClr val="phClr"><a:shade val="60000"/></a:schemeClr></a:gs>
</a:gsLst><a:lin ang="5400000" scaled="0"/></a:gradFill>
<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
</a:fillStyleLst>
<a:lnStyleLst>
<a:ln w="6350" cap="flat"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/></a:ln>
<a:ln w="25400" cap="flat"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/></a:ln>
<a:ln w="38100" cap="flat"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/></a:ln>
</a:lnStyleLst>
<a:effectStyleLst>
<a:effectStyle><a:effectLst/></a:effectStyle>
<a:effectStyle><a:effectLst/></a:effectStyle>
<a:effectStyle><a:effectLst/></a:effectStyle>
</a:effectStyleLst>
<a:bgFillStyleLst>
<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
</a:bgFillStyleLst>
</a:fmtScheme>
</a:themeElements>
</a:theme>'''

STYLE_REF = ('<xdr:style>'
             '<a:lnRef idx="2"><a:schemeClr val="accent1"/></a:lnRef>'
             '<a:fillRef idx="1"><a:schemeClr val="accent1"/></a:fillRef>'
             '<a:effectRef idx="0"><a:schemeClr val="accent1"/></a:effectRef>'
             '<a:fontRef idx="minor"><a:schemeClr val="lt1"/></a:fontRef>'
             '</xdr:style>')


def anchor(col, row, cx, cy, body):
    return (f'<xdr:oneCellAnchor>'
            f'<xdr:from><xdr:col>{col}</xdr:col><xdr:colOff>0</xdr:colOff>'
            f'<xdr:row>{row}</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>'
            f'<xdr:ext cx="{cx}" cy="{cy}"/>{body}<xdr:clientData/></xdr:oneCellAnchor>')


def shape(ident, name, cx, cy, geom, ink, style='', xfrm=''):
    return (f'<xdr:sp macro="" textlink=""><xdr:nvSpPr>'
            f'<xdr:cNvPr id="{ident}" name="{name}"/><xdr:cNvSpPr/></xdr:nvSpPr>'
            f'<xdr:spPr><a:xfrm{xfrm}><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'{geom}{ink}</xdr:spPr>{style}</xdr:sp>')


def drawing():
    parts = []

    # 1 — own fill and own line, no text at all.
    parts.append(anchor(0, 3, 1828800, 1371600, shape(
        2, 'Inked Star', 1828800, 1371600,
        '<a:prstGeom prst="star5"><a:avLst/></a:prstGeom>',
        '<a:solidFill><a:srgbClr val="FF0000"/></a:solidFill>'
        '<a:ln w="28575"><a:solidFill><a:srgbClr val="0000FF"/></a:solidFill></a:ln>')))

    # 2 — nothing stated; the theme's first fill style and second line style decide.
    parts.append(anchor(4, 3, 1828800, 914400, shape(
        3, 'Themed Box', 1828800, 914400,
        '<a:prstGeom prst="roundRect"><a:avLst/></a:prstGeom>', '', STYLE_REF)))

    # 3 — a:noFill and a:ln/a:noFill under the same style reference: the control.
    parts.append(anchor(8, 3, 1828800, 914400, shape(
        4, 'Bare Box', 1828800, 914400,
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>',
        '<a:noFill/><a:ln><a:noFill/></a:ln>', STYLE_REF)))

    # 4 — a quarter turn, whose anchor Excel writes for the *turned* rectangle.
    parts.append(anchor(0, 10, 2286000, 457200, shape(
        5, 'Turned Elbow', 457200, 2286000,
        '<a:prstGeom prst="bentConnector3"><a:avLst>'
        '<a:gd name="adj1" fmla="val 50000"/></a:avLst></a:prstGeom>',
        '<a:ln w="19050"><a:solidFill><a:srgbClr val="008000"/></a:solidFill></a:ln>',
        xfrm=' rot="5400000"')))

    # 5 — a group whose second child says a:grpFill and takes the group's own.
    group = ('<xdr:grpSp><xdr:nvGrpSpPr>'
             '<xdr:cNvPr id="6" name="Pair"/><xdr:cNvGrpSpPr/></xdr:nvGrpSpPr>'
             '<xdr:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="1828800" cy="914400"/>'
             '<a:chOff x="0" y="0"/><a:chExt cx="1828800" cy="914400"/></a:xfrm>'
             '<a:solidFill><a:srgbClr val="00A0A0"/></a:solidFill></xdr:grpSpPr>'
             + shape(7, 'Left', 914400, 914400,
                     '<a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>',
                     '<a:solidFill><a:srgbClr val="FFA500"/></a:solidFill>')
             .replace('<a:off x="0" y="0"/>', '<a:off x="0" y="0"/>')
             + shape(8, 'Right', 914400, 914400,
                     '<a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>',
                     '<a:grpFill/>')
             .replace('<a:off x="0" y="0"/>', '<a:off x="914400" y="0"/>')
             + '</xdr:grpSp>')
    parts.append(anchor(4, 10, 1828800, 914400, group))

    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"'
            ' xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
            + ''.join(parts) + '</xdr:wsDr>')


def main(outdir):
    path = os.path.join(outdir, 'sheet-shape-ink.xlsx')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CONTENT_TYPES)
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('xl/workbook.xml', WORKBOOK)
        z.writestr('xl/_rels/workbook.xml.rels', WORKBOOK_RELS)
        z.writestr('xl/styles.xml', STYLES)
        z.writestr('xl/theme/theme1.xml', THEME)
        z.writestr('xl/worksheets/sheet1.xml', SHEET)
        z.writestr('xl/worksheets/_rels/sheet1.xml.rels', SHEET_RELS)
        z.writestr('xl/drawings/drawing1.xml', drawing())
    print(path)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '.')
