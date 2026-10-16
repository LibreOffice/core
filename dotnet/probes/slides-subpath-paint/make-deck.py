#!/usr/bin/env python3
"""Build a one-slide deck of presets whose subpaths disagree about fill and stroke.

Each shape states a solid fill *and* a pen, so a subpath declaring `fill="none"` or
`stroke="false"` is visible as ink that should not be there. Five of the eight presets carry such
a subpath and three do not, which is the control: the three must not move.

    make-deck.py <out.pptx>
"""
import sys
import zipfile

SHAPES = [
    # name, preset, column, row   (three columns of three)
    ('bentConnector3', 'bentConnector3'),
    ('curvedConnector3', 'curvedConnector3'),
    ('foldedCorner', 'foldedCorner'),
    ('cube', 'cube'),
    ('can', 'can'),
    ('rect', 'rect'),
    ('ellipse', 'ellipse'),
    ('diamond', 'diamond'),
    ('homePlate', 'homePlate'),
]

EMU_IN = 914400


def shape_xml(index, name, preset):
    column, row = index % 3, index // 3
    x = int(0.4 * EMU_IN) + column * int(3.0 * EMU_IN)
    y = int(0.4 * EMU_IN) + row * int(2.2 * EMU_IN)
    return (
        f'<p:sp><p:nvSpPr><p:cNvPr id="{index + 2}" name="{name}"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr>'
        f'<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/>'
        f'<a:ext cx="{int(2.4 * EMU_IN)}" cy="{int(1.6 * EMU_IN)}"/></a:xfrm>'
        f'<a:prstGeom prst="{preset}"><a:avLst/></a:prstGeom>'
        '<a:solidFill><a:srgbClr val="4472C4"/></a:solidFill>'
        '<a:ln w="28575"><a:solidFill><a:srgbClr val="C00000"/></a:solidFill></a:ln>'
        '</p:spPr><p:txBody><a:bodyPr/><a:lstStyle/><a:p/></p:txBody></p:sp>')


SLIDE = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"'
    ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
    ' xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main"><p:cSld><p:spTree>'
    '<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>'
    '<p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/>'
    '<a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr>'
    '{shapes}</p:spTree></p:cSld><p:clrMapOvr><a:overrideClrMapping bg1="lt1" tx1="dk1" bg2="lt2"'
    ' tx2="dk2" accent1="accent1" accent2="accent2" accent3="accent3" accent4="accent4"'
    ' accent5="accent5" accent6="accent6" hlink="hlink" folHlink="folHlink"/></p:clrMapOvr></p:sld>')

PRESENTATION = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<p:presentation xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"'
    ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
    ' xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">'
    '<p:sldIdLst><p:sldId id="256" r:id="rId2"/></p:sldIdLst>'
    '<p:sldSz cx="9144000" cy="6858000"/><p:notesSz cx="6858000" cy="9144000"/></p:presentation>')

TYPES = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
    '<Default Extension="xml" ContentType="application/xml"/>'
    '<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>'
    '<Override PartName="/ppt/slides/slide1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>'
    '<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>'
    '</Types>')

ROOT_RELS = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="ppt/presentation.xml"/>'
    '</Relationships>')

PRES_RELS = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/>'
    '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" Target="slides/slide1.xml"/>'
    '</Relationships>')


def theme():
    scheme = ''.join(
        f'<a:{tag}><a:srgbClr val="{value}"/></a:{tag}>'
        for tag, value in [
            ('dk1', '000000'), ('lt1', 'FFFFFF'), ('dk2', '44546A'), ('lt2', 'E7E6E6'),
            ('accent1', '4472C4'), ('accent2', 'ED7D31'), ('accent3', 'A5A5A5'),
            ('accent4', 'FFC000'), ('accent5', '5B9BD5'), ('accent6', '70AD47'),
            ('hlink', '0563C1'), ('folHlink', '954F72')])
    fills = ('<a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
             '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
             '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>')
    lines = ('<a:lnStyleLst>' + '<a:ln w="6350"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
             '</a:ln>' * 3 + '</a:lnStyleLst>')
    effects = '<a:effectStyleLst>' + '<a:effectStyle><a:effectLst/></a:effectStyle>' * 3 + '</a:effectStyleLst>'
    backgrounds = ('<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
                   '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
                   '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>')
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="probe">'
        f'<a:themeElements><a:clrScheme name="probe">{scheme}</a:clrScheme>'
        '<a:fontScheme name="probe"><a:majorFont><a:latin typeface="Liberation Sans"/>'
        '<a:ea typeface=""/><a:cs typeface=""/></a:majorFont><a:minorFont>'
        '<a:latin typeface="Liberation Sans"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont>'
        '</a:fontScheme><a:fmtScheme name="probe">'
        f'{fills}{lines}{effects}{backgrounds}</a:fmtScheme></a:themeElements></a:theme>')


def main(destination):
    shapes = ''.join(shape_xml(i, name, preset) for i, (name, preset) in enumerate(SHAPES))
    with zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED) as package:
        package.writestr('[Content_Types].xml', TYPES)
        package.writestr('_rels/.rels', ROOT_RELS)
        package.writestr('ppt/presentation.xml', PRESENTATION)
        package.writestr('ppt/_rels/presentation.xml.rels', PRES_RELS)
        package.writestr('ppt/theme/theme1.xml', theme())
        package.writestr('ppt/slides/slide1.xml', SLIDE.format(shapes=shapes))
    print(f'wrote {destination}')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else 'subpath-paint.pptx')
