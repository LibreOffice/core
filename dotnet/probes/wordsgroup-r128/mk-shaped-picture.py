#!/usr/bin/env python3
"""Writes the `picture-shaped*.docx` fixtures.

Three inline pictures of 288 x 216 pt, identical but for their `pic:spPr` geometry:

  * `picture-shaped.docx`      -- `a:prstGeom prst="ellipse"`, no crop;
  * `picture-shaped-crop.docx` -- the same ellipse plus the `a:srcRect` of
    `picture-crop.docx`, so the crop and the outline have to compose;
  * `picture-shaped-rect.docx` -- `prst="rect"`, the control that must stay unclipped.

The image part is the one `picture-crop.docx` already carries, so the fixtures add no
new licensed bytes to the repository.

Usage: mk-shaped-picture.py <existing picture-crop.docx> <outdir>
"""
import os
import sys
import zipfile

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>
'''

DOC_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId9" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/image1.png"/>
</Relationships>
'''

TYPES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>
'''

DOCUMENT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture">
<w:body>
<w:p><w:r><w:drawing>
<wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="3657600" cy="2743200"/>
<wp:docPr id="1" name="Shaped"/>
<a:graphic><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">
<pic:pic><pic:nvPicPr><pic:cNvPr id="1" name="Shaped"/><pic:cNvPicPr/></pic:nvPicPr>
<pic:blipFill><a:blip r:embed="rId9"/>{crop}<a:stretch><a:fillRect/></a:stretch></pic:blipFill>
<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="3657600" cy="2743200"/></a:xfrm>
<a:prstGeom prst="{preset}"><a:avLst/></a:prstGeom></pic:spPr>
</pic:pic></a:graphicData></a:graphic>
</wp:inline>
</w:drawing></w:r></w:p>
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>
</w:sectPr>
</w:body></w:document>
'''

CROP = '<a:srcRect l="10000" t="20000" r="30000" b="40000"/>'

FIXTURES = {
    'picture-shaped.docx': ('ellipse', ''),
    'picture-shaped-crop.docx': ('ellipse', CROP),
    'picture-shaped-rect.docx': ('rect', ''),
}


def main():
    source, outdir = sys.argv[1], sys.argv[2]
    with zipfile.ZipFile(source) as z:
        image = z.read('word/media/image1.png')
    os.makedirs(outdir, exist_ok=True)
    for name, (preset, crop) in FIXTURES.items():
        path = os.path.join(outdir, name)
        with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
            z.writestr('[Content_Types].xml', TYPES)
            z.writestr('_rels/.rels', RELS)
            z.writestr('word/_rels/document.xml.rels', DOC_RELS)
            z.writestr('word/media/image1.png', image)
            z.writestr('word/document.xml',
                       DOCUMENT.format(preset=preset, crop=crop))
        print(path, os.path.getsize(path))


main()
