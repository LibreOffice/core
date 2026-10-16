#!/usr/bin/env python3
"""One-page DOCX fixtures that isolate what each `relativeFrom` value measures from.

`wp:positionH`/`wp:positionV`'s `relativeFrom` has nine values between the two axes and
`PositionHandler::lcl_attribute` (`sw/source/writerfilter/dmapper/GraphicHelpers.cxx`:61-133)
maps each to a `RelOrientation` the layout then resolves in
`SwAnchoredObjectPosition::GetVertAlignmentValues` / `GetHoriAlignmentValues`
(`sw/source/core/objectpositioning/anchoredobjectposition.cxx`:280-370 and :735-871). Three of
those mappings are counter-intuitive enough that only a measurement settles them:

* `topMargin` shares the `PAGE_FRAME` case, so it is the whole page rather than the top margin;
* `bottomMargin` is the band from the *body's* bottom to the page's;
* `outsideMargin` is not handled at all and keeps the default, which is the text column.

VML states the same thing differently, and its own mapping
(`lcl_SetAnchorType`, `oox/source/vml/vmlshape.cxx`:616-700) crosses two of them over:
`inner-margin-area` is `PAGE_RIGHT` and `outer-margin-area` is `PAGE_LEFT`.

Each fixture is one A4 page carrying a red band, a one-line header and a one-line footer, both
with room reserved so the body's own rectangle is exactly `w:top`..`pageHeight - w:bottom` and
the header-overflow rule measured in `probes/words-margin-print-area/` cannot confound this one.

Usage:  python3 make-relfrom.py <outdir>
"""
import os, sys, zipfile

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
      '<Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>'
      '<Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>'
      '</Types>')

RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
        '</Relationships>')

DRELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
         '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
         '<Relationship Id="rIdH" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header" Target="header1.xml"/>'
         '<Relationship Id="rIdF" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" Target="footer1.xml"/>'
         '</Relationships>')

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
      'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" '
      'xmlns:v="urn:schemas-microsoft-com:vml" '
      'xmlns:w10="urn:schemas-microsoft-com:office:word" '
      'xmlns:o="urn:schemas-microsoft-com:office:office"')

RPR = '<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>'

EMU_PER_PT = 12700


def dml(width_pt, height_pt, hrel, hpos, vrel, vpos):
    """A `wp:anchor` band. `hpos`/`vpos` are either a number of points or an align keyword."""
    def place(tag, rel, pos):
        inner = (f'<wp:align>{pos}</wp:align>' if isinstance(pos, str)
                 else f'<wp:posOffset>{round(pos * EMU_PER_PT)}</wp:posOffset>')
        return f'<wp:{tag} relativeFrom="{rel}">{inner}</wp:{tag}>'

    cx, cy = round(width_pt * EMU_PER_PT), round(height_pt * EMU_PER_PT)
    return (
        '<w:r><w:drawing><wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0"'
        ' relativeHeight="1" behindDoc="1" locked="0" layoutInCell="0" allowOverlap="1">'
        '<wp:simplePos x="0" y="0"/>'
        + place('positionH', hrel, hpos) + place('positionV', vrel, vpos)
        + f'<wp:extent cx="{cx}" cy="{cy}"/>'
        '<wp:effectExtent l="0" t="0" r="0" b="0"/><wp:wrapNone/>'
        '<wp:docPr id="9" name="band"/>'
        '<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">'
        f'<wps:wsp><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
        '<a:solidFill><a:srgbClr val="FF0000"/></a:solidFill>'
        '<a:ln w="0"><a:noFill/></a:ln></wps:spPr><wps:bodyPr/></wps:wsp>'
        '</a:graphicData></a:graphic></wp:anchor></w:drawing></w:r>')


def vml(width_pt, height_pt, hrel, hpos, vrel, vpos):
    """The same band as a `v:rect`, positioned by `mso-position-*-relative`."""
    style = (f'position:absolute;margin-left:{hpos}pt;margin-top:{vpos}pt;'
             f'width:{width_pt}pt;height:{height_pt}pt;z-index:-251658240;'
             f'mso-position-horizontal-relative:{hrel};'
             f'mso-position-vertical-relative:{vrel}')
    return ('<w:r><w:pict>'
            f'<v:rect id="band" style="{style}" fillcolor="#ff0000" stroked="f">'
            '<w10:wrap type="none"/></v:rect></w:pict></w:r>')


def build(path, band):
    header = f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:hdr {NS}><w:p><w:r>{RPR}<w:t>HDR</w:t></w:r></w:p></w:hdr>'
    footer = f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:ftr {NS}><w:p><w:r>{RPR}<w:t>FTR</w:t></w:r></w:p></w:ftr>'
    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {NS}><w:body>'
        f'<w:p>{band}<w:r>{RPR}<w:t>BODYLINE</w:t></w:r></w:p>'
        '<w:sectPr><w:headerReference w:type="default" r:id="rIdH"/>'
        '<w:footerReference w:type="default" r:id="rIdF"/>'
        '<w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" '
        'w:header="720" w:footer="720" w:gutter="0"/></w:sectPr></w:body></w:document>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)
        z.writestr('word/_rels/document.xml.rels', DRELS)
        z.writestr('word/header1.xml', header)
        z.writestr('word/footer1.xml', footer)


# A tall thin band for the vertical fixtures and a short wide one for the horizontal ones, so
# neither ever runs off the page in the axis that is not under test.
VW, VH = 60.0, 20.0
HW, HH = 40.0, 20.0

VERTICAL = [
    # name                       relativeFrom      posOffset / align
    ('v-page-0',                 'page',           0),
    ('v-margin-0',               'margin',         0),
    ('v-topmargin-0',            'topMargin',      0),
    ('v-topmargin-36',           'topMargin',      36),
    ('v-topmargin-top',          'topMargin',      'top'),
    ('v-topmargin-centre',       'topMargin',      'center'),
    ('v-topmargin-bottom',       'topMargin',      'bottom'),
    ('v-bottommargin-0',         'bottomMargin',   0),
    ('v-bottommargin-36',        'bottomMargin',   36),
    ('v-bottommargin-top',       'bottomMargin',   'top'),
    ('v-bottommargin-centre',    'bottomMargin',   'center'),
    ('v-bottommargin-bottom',    'bottomMargin',   'bottom'),
]

HORIZONTAL = [
    ('h-page-0',                 'page',           0),
    ('h-margin-0',               'margin',         0),
    ('h-column-0',               'column',         0),
    ('h-leftmargin-0',           'leftMargin',     0),
    ('h-leftmargin-right',       'leftMargin',     'right'),
    ('h-leftmargin-centre',      'leftMargin',     'center'),
    ('h-rightmargin-0',          'rightMargin',    0),
    ('h-rightmargin-left',       'rightMargin',    'left'),
    ('h-rightmargin-right',      'rightMargin',    'right'),
    ('h-rightmargin-centre',     'rightMargin',    'center'),
    ('h-insidemargin-0',         'insideMargin',   0),
    ('h-outsidemargin-0',        'outsideMargin',  0),
    ('h-outsidemargin-right',    'outsideMargin',  'right'),
]

# VML's own spellings, on the same page. `margin-left`/`margin-top` are the offsets.
VML = [
    ('m-v-topmargin-0',      'page',              0, 'top-margin-area',    0),
    ('m-v-topmargin-36',     'page',              0, 'top-margin-area',    36),
    ('m-v-bottommargin-0',   'page',              0, 'bottom-margin-area', 0),
    ('m-v-margin-0',         'page',              0, 'margin',             0),
    ('m-h-leftmargin-0',     'left-margin-area',  0, 'page',               300),
    ('m-h-rightmargin-0',    'right-margin-area', 0, 'page',               300),
    ('m-h-innermargin-0',    'inner-margin-area', 0, 'page',               300),
    ('m-h-outermargin-0',    'outer-margin-area', 0, 'page',               300),
    ('m-h-margin-0',         'margin',            0, 'page',               300),
]


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    for name, rel, pos in VERTICAL:
        build(os.path.join(out, name + '.docx'),
              dml(VW, VH, 'page', 100.0, rel, pos))
    for name, rel, pos in HORIZONTAL:
        build(os.path.join(out, name + '.docx'),
              dml(HW, HH, rel, pos, 'page', 300.0))
    for name, hrel, hpos, vrel, vpos in VML:
        build(os.path.join(out, name + '.docx'),
              vml(HW, HH, hrel, hpos, vrel, vpos))
    print('built', len(VERTICAL) + len(HORIZONTAL) + len(VML), 'fixtures in', out)
