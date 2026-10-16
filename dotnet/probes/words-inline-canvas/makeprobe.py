#!/usr/bin/env python3
"""Build DOCX fixtures holding an inline drawing **canvas** (`wpc:wpc`) and an inline
**group** (`wpg:wgp`), each with three rectangles at known offsets inside it.

A canvas is a drawing with its own coordinate space: the members state `a:off` inside
it and the canvas itself states only `wp:extent`. A group states an `a:chOff`/`a:chExt`
child space as well, so the two exercise different halves of the same reader.

Each rectangle gets its own primary colour, so where each one landed can be read out
of the raster without pairing anything.

Usage:  python3 makeprobe.py <outdir>
"""
import os, sys, zipfile

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
      '</Types>')

RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
        '</Relationships>')

RPR = ('<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
       '<w:sz w:val="24"/></w:rPr>')

# 914400 EMU to the inch. The canvas is 4 x 2 in; the three members are 1 x 0.5 in each,
# stepped diagonally so that no two share a row or a column.
MEMBERS = [('FF0000', 0, 0), ('00FF00', 1371600, 457200), ('0000FF', 2743200, 914400)]
MW, MH = 914400, 457200
CW, CH = 3657600, 1828800


def wsp(colour, x, y):
    return (f'<wps:wsp><wps:cNvPr id="0" name="m{colour}"/><wps:cNvSpPr/><wps:spPr>'
            f'<a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{MW}" cy="{MH}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
            f'<a:solidFill><a:srgbClr val="{colour}"/></a:solidFill>'
            f'<a:ln w="0"><a:noFill/></a:ln></wps:spPr><wps:bodyPr/></wps:wsp>')


BODIES = ''.join(wsp(*m) for m in MEMBERS)

CANVAS = ('<wpc:wpc xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas">'
          '<wpc:bg><a:noFill/></wpc:bg><wpc:whole/>' + BODIES + '</wpc:wpc>')

GROUP = ('<wpg:wgp xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup">'
         '<wpg:cNvGrpSpPr/><wpg:grpSpPr>'
         f'<a:xfrm><a:off x="0" y="0"/><a:ext cx="{CW}" cy="{CH}"/>'
         f'<a:chOff x="0" y="0"/><a:chExt cx="{CW}" cy="{CH}"/></a:xfrm>'
         '</wpg:grpSpPr>' + BODIES + '</wpg:wgp>')

URI = {'canvas': 'http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas',
       'group': 'http://schemas.microsoft.com/office/word/2010/wordprocessingGroup'}

DOC = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<w:body>
<w:p><w:r>{rpr}<w:t>TOPLINE</w:t></w:r></w:p>
<w:p><w:r>{rpr}
<w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="{cw}" cy="{ch}"/>
<wp:effectExtent l="0" t="0" r="0" b="0"/>
<wp:docPr id="1" name="probe"/>
<a:graphic><a:graphicData uri="{uri}">
{body}</a:graphicData></a:graphic></wp:inline></w:drawing>
</w:r></w:p>
<w:p><w:r>{rpr}<w:t>BOTLINE</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr></w:body></w:document>'''


def build(path, kind, body):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml',
                   DOC.format(rpr=RPR, uri=URI[kind], body=body, cw=CW, ch=CH))


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    build(os.path.join(out, 'canvas.docx'), 'canvas', CANVAS)
    build(os.path.join(out, 'group.docx'), 'group', GROUP)
    print('built in', out)
