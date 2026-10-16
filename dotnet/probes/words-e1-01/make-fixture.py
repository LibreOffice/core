#!/usr/bin/env python3
"""Author `tests/corpus/features/header-behind-text.docx`.

One page, one line of body text, and a header holding two anchored shapes:

  * a full-page rectangle with a 50% black `a:solidFill` and `wp:wrapNone`, which is the
    letterhead shape — Word's own way of writing a background, and the thing
    `words/extra-001/doc/info-bulletin-601.doc` carries as a raster;
  * a small box with a `wrapSquare` wrap, an opaque `a:solidFill` and a `a:ln` outline, which
    is the appearance case: a shape whose fill and border we drew as nothing at all.

Authored rather than cut down from a corpus document so that the two questions — paint order
and appearance — are separable, and so that the expected answer is arithmetic rather than a
reading of somebody's letterhead.
"""
import os, sys, zipfile

W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main"
R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml" ContentType="application/xml"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
  <Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
  <Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>"""

ROOT_RELS = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOC_RELS = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="{R}/header" Target="header1.xml"/>
  <Relationship Id="rId2" Type="{R}/settings" Target="settings.xml"/>
</Relationships>"""

# compatibilityMode 15 — Word 2013 — so that the `wrapNone` exception in tdf#137850 is the branch
# under test rather than the pre-2013 blanket honouring of behindDoc.
SETTINGS = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="{W}">
  <w:compat>
    <w:compatSetting w:name="compatibilityMode"
                     w:uri="http://schemas.microsoft.com/office/word"
                     w:val="15"/>
  </w:compat>
</w:settings>"""

# A4 in twips: 11906 x 16838, one-inch margins.
DOCUMENT = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="{W}" xmlns:r="{R}">
  <w:body>
    <w:p><w:r><w:t>BODYLINE</w:t></w:r></w:p>
    <w:sectPr>
      <w:headerReference w:type="default" r:id="rId1"/>
      <w:pgSz w:w="11906" w:h="16838"/>
      <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
               w:header="708" w:footer="708" w:gutter="0"/>
    </w:sectPr>
  </w:body>
</w:document>"""


def shape(name, cx, cy, wrap, behind, fill, line):
    """One `w:drawing` holding a single `wps:wsp` with the stated fill and outline."""
    attr = ' behindDoc="1"' if behind else ' behindDoc="0"'
    ln = (f'<a:ln w="{line[1]}"><a:solidFill><a:srgbClr val="{line[0]}"/></a:solidFill></a:ln>'
          if line else "<a:ln><a:noFill/></a:ln>")
    return f"""<w:r><w:drawing>
      <wp:anchor xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
                 distT="0" distB="0" distL="0" distR="0"
                 simplePos="0" relativeHeight="1" allowOverlap="1"{attr}>
        <wp:simplePos x="0" y="0"/>
        <wp:positionH relativeFrom="page"><wp:posOffset>0</wp:posOffset></wp:positionH>
        <wp:positionV relativeFrom="page"><wp:posOffset>0</wp:posOffset></wp:positionV>
        <wp:extent cx="{cx}" cy="{cy}"/>
        <wp:{wrap}/>
        <wp:docPr id="1" name="{name}"/>
        <a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
          <a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
            <wps:wsp xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
              <wps:spPr>
                <a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
                <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                {fill}
                {ln}
              </wps:spPr>
              <wps:bodyPr/>
            </wps:wsp>
          </a:graphicData>
        </a:graphic>
      </wp:anchor>
    </w:drawing></w:r>"""


# A4 is 7560000 x 10692000 EMUs.
PANEL = shape("Letterhead", 7560000, 10692000, "wrapNone", True,
              '<a:solidFill><a:srgbClr val="000000"><a:alpha val="50000"/></a:srgbClr></a:solidFill>',
              None)

# 2 cm square, a wrap that leaves a hole, an opaque fill and a 1 pt outline.
BOX = shape("Panel", 720000, 720000, "wrapSquare", False,
            '<a:solidFill><a:srgbClr val="777777"/></a:solidFill>',
            ("FF0000", 12700))

HEADER = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:hdr xmlns:w="{W}" xmlns:r="{R}">
  <w:p>{PANEL}{BOX}</w:p>
</w:hdr>"""


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        here, "..", "..", "tests", "corpus", "features", "header-behind-text.docx")
    out = os.path.abspath(out)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/document.xml", DOCUMENT)
        z.writestr("word/header1.xml", HEADER)
        z.writestr("word/settings.xml", SETTINGS)
    print(out)


if __name__ == "__main__":
    main()
