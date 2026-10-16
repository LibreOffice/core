#!/usr/bin/env python3
"""One-page DOCX fixtures that settle *which* anchored objects 26.2.4.2 captures, and in what.

`SwAnchoredObjectPosition`'s constructor (`anchoredobjectposition.cxx`:125-144) sets

    mbDoNotCaptureAnchoredObj = bConsidered && !mbFollowTextFlow && DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE
    fly  (picture, OLE, text frame)  bConsidered = bWrapThrough && !bTextBox
    draw (shape, group)              bConsidered = bWrapThrough || !bTextBox

and `ImplAdjustVertRelPos` (:552-573) then narrows the area it is held inside from the page to the
page *body* under DOCX `compatibilityMode` 15, for every vertical relation but `PAGE_FRAME` and
`PAGE_PRINT_AREA` and only where the anchor has a body frame at all.

Every fixture is one A4 page with 72 pt margins carrying a single red band, which is the only
coloured ink on it, so `measure-capture.py` can read the band's own rectangle off the raster with
no assumption about the text. The axes are

    kind   shape   a bare `wps:wsp`            -> a draw object with no text box
           text    a `wps:wsp` with `wps:txbx` -> a draw object that is half of a TextBox pair
           pic     a `pic:pic`                 -> a fly
    wrap   none square tight through topAndBottom
    where  fit     inside the page and the body: the control, which must never move
           hleft   `positionH relativeFrom="page" posOffset="-40pt"`  -> off the sheet's left
           vabove  `positionV relativeFrom="page" posOffset="-30pt"`  -> off the sheet's top
           vbody   `positionV relativeFrom="paragraph" posOffset="-52pt"` -> on the sheet, above
                   the body's top, and under a relation the narrowing does *not* exclude

`hleft` and `vabove` ask whether the object is captured at all; `vbody` asks whether the area is
the sheet or the body, and is written twice — with `w:compatibilityMode w:val="15"` and with 14 —
so the guard can be read off the pair.

Usage:  python3 make-capture.py <outdir>
"""
import pathlib
import struct
import sys
import zlib
import zipfile

EMU = 12700
PAGE_W, PAGE_H, MARGIN = 595.3, 841.9, 72.0
BAND_W, BAND_H = 100.0, 40.0

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Default Extension="png" ContentType="image/png"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
      '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>'
      '<Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>'
      '</Types>')

RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
        '</Relationships>')

DRELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
         '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
         '<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>'
         '<Relationship Id="rIdI" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/red.png"/>'
         '<Relationship Id="rIdH" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header" Target="header1.xml"/>'
         '</Relationships>')

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
      'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture" '
      'xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" '
      'xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" '
      'xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"')

RPR = '<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>'


def red_png():
    """A 4 x 4 opaque red PNG, so a `pic:pic` band is the same colour as a shape's fill."""
    raw = b''.join(b'\x00' + b'\xff\x00\x00' * 4 for _ in range(4))

    def chunk(tag, body):
        return (struct.pack('>I', len(body)) + tag + body
                + struct.pack('>I', zlib.crc32(tag + body) & 0xffffffff))

    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', struct.pack('>IIBBBBB', 4, 4, 8, 2, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(raw))
            + chunk(b'IEND', b''))


def body_of(kind, cx, cy):
    """The `a:graphicData` for one of the three object kinds, all filled the same red."""
    if kind == 'pic':
        return (
            '<a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">'
            '<pic:pic><pic:nvPicPr><pic:cNvPr id="1" name="band"/><pic:cNvPicPr/></pic:nvPicPr>'
            '<pic:blipFill><a:blip r:embed="rIdI"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>'
            f'<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData>')

    txbx = ('<wps:txbx><w:txbxContent><w:p><w:r>'
            '<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
            '<w:sz w:val="20"/></w:rPr><w:t>IN</w:t></w:r></w:p></w:txbxContent></wps:txbx>'
            ) if kind == 'text' else ''

    return (
        '<a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">'
        '<wps:wsp><wps:cNvSpPr/><wps:spPr>'
        f'<a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
        '<a:solidFill><a:srgbClr val="FF0000"/></a:solidFill>'
        '<a:ln w="0"><a:noFill/></a:ln></wps:spPr>'
        + txbx + '<wps:bodyPr/></wps:wsp></a:graphicData>')


WRAPS = {
    'none': '<wp:wrapNone/>',
    'square': '<wp:wrapSquare wrapText="bothSides"/>',
    'tight': '<wp:wrapTight wrapText="bothSides"><wp:wrapPolygon edited="0">'
             '<wp:start x="0" y="0"/><wp:lineTo x="0" y="21600"/>'
             '<wp:lineTo x="21600" y="21600"/><wp:lineTo x="21600" y="0"/>'
             '<wp:lineTo x="0" y="0"/></wp:wrapPolygon></wp:wrapTight>',
    'through': '<wp:wrapThrough wrapText="bothSides"><wp:wrapPolygon edited="0">'
               '<wp:start x="0" y="0"/><wp:lineTo x="0" y="21600"/>'
               '<wp:lineTo x="21600" y="21600"/><wp:lineTo x="21600" y="0"/>'
               '<wp:lineTo x="0" y="0"/></wp:wrapPolygon></wp:wrapThrough>',
    'topAndBottom': '<wp:wrapTopAndBottom/>',
}

# (horizontal relation, offset), (vertical relation, offset)
WHERE = {
    'fit': (('page', 200.0), ('page', 300.0)),
    'hleft': (('page', -40.0), ('page', 300.0)),
    'vabove': (('page', 200.0), ('page', -30.0)),
    'vbody': (('page', 200.0), ('paragraph', -52.0)),
    # Off the sheet's right and bottom edges. These are the discriminating form of "is it captured
    # at all": a band hanging off the *left* is clipped by the page, so its drawn left edge is
    # 0.00 whether it was clamped or not, and the first cut of this probe read twenty-five such
    # rows as agreement when they carry no information. Hanging it off the far edge instead moves
    # the corner the raster can see.
    'hright': (('page', PAGE_W - 40.0), ('page', 300.0)),
    'vbelow': (('page', 200.0), ('page', PAGE_H - 20.0)),
    # Anchored in the running head, at its own paragraph: on the sheet, above the body's top, and
    # under a relation the narrowing does not exclude — so the only thing that keeps it where the
    # file states it is `mpAnchorFrame->FindBodyFrame()` finding nothing. `b053-19` and
    # `Case-Study-Heathrow-Airport` are this case and are what the wide rule of
    # `probes/frame-area-r85` cost the most.
    'hdr': (('page', 200.0), ('paragraph', 0.0)),
}


def anchor(kind, wrap, where):
    (hrel, hoff), (vrel, voff) = WHERE[where]
    cx, cy = round(BAND_W * EMU), round(BAND_H * EMU)
    return (
        '<w:r><w:drawing><wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0"'
        ' relativeHeight="251658240" behindDoc="0" locked="0" layoutInCell="0" allowOverlap="1">'
        '<wp:simplePos x="0" y="0"/>'
        f'<wp:positionH relativeFrom="{hrel}"><wp:posOffset>{round(hoff * EMU)}</wp:posOffset></wp:positionH>'
        f'<wp:positionV relativeFrom="{vrel}"><wp:posOffset>{round(voff * EMU)}</wp:posOffset></wp:positionV>'
        f'<wp:extent cx="{cx}" cy="{cy}"/><wp:effectExtent l="0" t="0" r="0" b="0"/>'
        + WRAPS[wrap] +
        '<wp:docPr id="9" name="band"/><a:graphic>' + body_of(kind, cx, cy)
        + '</a:graphic></wp:anchor></w:drawing></w:r>')


def build(path, kind, wrap, where, compat):
    settings = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                f'<w:settings {NS}><w:compat>'
                f'<w:compatSetting w:name="compatibilityMode" '
                f'w:uri="http://schemas.microsoft.com/office/word" w:val="{compat}"/>'
                '</w:compat></w:settings>')
    band = anchor(kind, wrap, where)
    in_header = where == 'hdr'
    header = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:hdr {NS}><w:p>{band if in_header else ""}'
              f'<w:r>{RPR}<w:t>HDR</w:t></w:r></w:p></w:hdr>')
    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {NS}><w:body>'
        f'<w:p>{"" if in_header else band}<w:r>{RPR}<w:t>BODYLINE</w:t></w:r></w:p>'
        f'<w:p><w:r>{RPR}<w:t>SECOND</w:t></w:r></w:p>'
        '<w:sectPr><w:headerReference w:type="default" r:id="rIdH"/>'
        '<w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" '
        'w:header="720" w:footer="720" w:gutter="0"/></w:sectPr></w:body></w:document>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)
        z.writestr('word/_rels/document.xml.rels', DRELS)
        z.writestr('word/settings.xml', settings)
        z.writestr('word/header1.xml', header)
        z.writestr('word/media/red.png', red_png())


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    names = []
    for kind in ('shape', 'text', 'pic'):
        for wrap in WRAPS:
            for where in WHERE:
                # The three added last discriminate one thing each and are not worth five wraps.
                if where in ('hright', 'vbelow', 'hdr') and wrap not in ('none', 'square'):
                    continue
                name = f'{kind}-{wrap}-{where}-c15'
                build(out / f'{name}.docx', kind, wrap, where, 15)
                names.append(name)
                if where == 'vbody':
                    name = f'{kind}-{wrap}-{where}-c14'
                    build(out / f'{name}.docx', kind, wrap, where, 14)
                    names.append(name)
    print(f'{len(names)} fixtures in {out}')


main()
