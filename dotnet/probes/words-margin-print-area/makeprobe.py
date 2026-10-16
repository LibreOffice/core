#!/usr/bin/env python3
"""Build one-page DOCX fixtures that isolate what a vertically margin-relative
frame is positioned *inside*.

`wp:anchor/wp:positionV/@relativeFrom="margin"` is `RelOrientation::PAGE_PRINT_AREA`
in Writer, and `anchoredobjectposition.cxx`:336-361 takes the page's print area and
then walks the page frame's lowers, subtracting each header frame's height from the
area and adding it to the offset, and subtracting each footer frame's height. So the
area is bounded by the header frame's *bottom* and the footer frame's *top* — which
equals `w:top`..`w:bottom` only while the running heads fit the room those reserve.

Each fixture is one page carrying a 200 x 50 pt black rectangle, centred vertically
against the margin, plus a `BODYLINE` run so the body's own top can be read at the
same time.

Usage:  python3 makeprobe.py <outdir>
"""
import os, sys, zipfile

CT_HEAD = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
           '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
           '<Default Extension="xml" ContentType="application/xml"/>'
           '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>')
CT_HDR = '<Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>'
CT_FTR = '<Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>'

RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
        '</Relationships>')

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
      'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape"')

# 200 x 50 pt, centred both ways against the margin, behind the text and wrapping nothing.
SHAPE = '''<w:r><w:drawing><wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0"
 relativeHeight="1" behindDoc="1" locked="0" layoutInCell="0" allowOverlap="1">
<wp:simplePos x="0" y="0"/>
<wp:positionH relativeFrom="margin"><wp:align>center</wp:align></wp:positionH>
<wp:positionV relativeFrom="margin"><wp:align>center</wp:align></wp:positionV>
<wp:extent cx="2540000" cy="635000"/>
<wp:effectExtent l="0" t="0" r="0" b="0"/>
<wp:wrapNone/>
<wp:docPr id="9" name="band"/>
<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<wps:wsp><wps:cNvSpPr/><wps:spPr>
<a:xfrm><a:off x="0" y="0"/><a:ext cx="2540000" cy="635000"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="000000"/></a:solidFill>
<a:ln w="0"><a:noFill/></a:ln>
</wps:spPr><wps:bodyPr/></wps:wsp></a:graphicData></a:graphic>
</wp:anchor></w:drawing></w:r>'''

RPR = '<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>'


def line(text):
    return f'<w:p><w:r>{RPR}<w:t>{text}</w:t></w:r></w:p>'


def build(path, header=None, footer=None, top=708, hdr=708, bottom=1440, ftr=708,
          shape_in='body'):
    parts = {}
    ct = CT_HEAD
    drels = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">']
    refs = ''

    if header is not None:
        ct += CT_HDR
        drels.append('<Relationship Id="rIdH" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header" Target="header1.xml"/>')
        refs += '<w:headerReference w:type="default" r:id="rIdH"/>'
        body = header + (SHAPE_P if shape_in == 'header' else '')
        parts['word/header1.xml'] = (
            f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:hdr {NS}>{body}</w:hdr>')
    if footer is not None:
        ct += CT_FTR
        drels.append('<Relationship Id="rIdF" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" Target="footer1.xml"/>')
        refs += '<w:footerReference w:type="default" r:id="rIdF"/>'
        parts['word/footer1.xml'] = (
            f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:ftr {NS}>{footer}</w:ftr>')

    drels.append('</Relationships>')
    ct += '</Types>'

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {NS}><w:body>'
        + (f'<w:p>{SHAPE}<w:r>{RPR}<w:t>BODYLINE</w:t></w:r></w:p>'
           if shape_in == 'body' else line('BODYLINE'))
        + f'<w:sectPr>{refs}<w:pgSz w:w="11906" w:h="16838"/>'
          f'<w:pgMar w:top="{top}" w:right="1440" w:bottom="{bottom}" w:left="1440" '
          f'w:header="{hdr}" w:footer="{ftr}" w:gutter="0"/></w:sectPr>'
        '</w:body></w:document>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ct)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)
        z.writestr('word/_rels/document.xml.rels', ''.join(drels))
        for name, content in parts.items():
            z.writestr(name, content)


SHAPE_P = f'<w:p>{SHAPE}</w:p>'

if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    P = lambda n: os.path.join(out, n + '.docx')

    # No running heads at all: the area is the body's and nothing can move it.
    build(P('none'))
    # An empty header with no room reserved for it — `w:top` == `w:header`, which is
    # ordinary in Word and is `DOA_Template`'s own shape.
    build(P('hdr-empty'), header=line(''))
    build(P('hdr-1line'), header=line('HEADERA'))
    build(P('hdr-3line'), header=line('HEADERA') + line('HEADERB') + line('HEADERC'))
    # Room reserved and not exceeded: `w:top` 2000 tw against `w:header` 708 leaves
    # 64.6 pt for a header needing about 14, so nothing overflows.
    build(P('hdr-roomy'), header=line('HEADERA'), top=2000)
    # The footer half of the same rule.
    build(P('ftr-3line'), footer=line('FOOTERA') + line('FOOTERB') + line('FOOTERC'))
    build(P('hdr3-ftr3'),
          header=line('HEADERA') + line('HEADERB') + line('HEADERC'),
          footer=line('FOOTERA') + line('FOOTERB') + line('FOOTERC'))
    # The corpus shape: the watermark lives in the header, not in the body.
    build(P('inhdr-3line'),
          header=line('HEADERA') + line('HEADERB') + line('HEADERC'), shape_in='header')
    print('built in', out)
