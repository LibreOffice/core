#!/usr/bin/env python3
"""Hand-built DOCX isolating the two halves of an as-character frame's horizontal placement.

Each document is one page, portrait Letter, 1 inch margins (text area 72..540 pt), and holds a
right-aligned paragraph whose only content is an inline picture 145.5 pt wide.

  align-only   the paragraph alone. Right alignment must put the picture's LEFT edge at
               540 - 145.5 = 394.5, not at 540.
  band         the same paragraph with a floating frame anchored in it, square wrap, whose
               left edge is 250 pt from the page. Right alignment inside the band left of the
               frame must put the picture's left edge at (frame left - its wrap distance) - 145.5.
  left-align   the control: left alignment must be unaffected by either question.

Run each through a reference binary and read the image rectangle out with pdf-ops.py.
"""
import os, struct, sys, zlib
from pathlib import Path

OUT = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
OUT.mkdir(parents=True, exist_ok=True)

def png(w, h, rgb=(0x20, 0x60, 0xC0)):
    raw = b"".join(b"\x00" + bytes(rgb) * w for _ in range(h))
    def chunk(tag, data):
        c = tag + data
        return struct.pack(">I", len(data)) + c + struct.pack(">I", zlib.crc32(c))
    return (b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw))
            + chunk(b"IEND", b""))

EMU_PT = 12700
PIC_W, PIC_H = int(145.5 * EMU_PT), int(68.5 * EMU_PT)

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
 <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
 <Default Extension="xml" ContentType="application/xml"/>
 <Default Extension="png" ContentType="image/png"/>
 <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>"""

ROOTRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
 <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOCRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
 <Relationship Id="rIdImg" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/logo.png"/>
</Relationships>"""

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
      'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture" '
      'xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" '
      'xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"')

def inline_pic():
    return f"""<w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">
      <wp:extent cx="{PIC_W}" cy="{PIC_H}"/><wp:effectExtent l="0" t="0" r="0" b="0"/>
      <wp:docPr id="9" name="Logo"/>
      <a:graphic><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">
        <pic:pic><pic:nvPicPr><pic:cNvPr id="9" name="Logo"/><pic:cNvPicPr/></pic:nvPicPr>
          <pic:blipFill><a:blip r:embed="rIdImg"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>
          <pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{PIC_W}" cy="{PIC_H}"/></a:xfrm>
            <a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>
      </a:graphicData></a:graphic></wp:inline></w:drawing>"""

def floating_frame(left_pt, width_pt, height_pt, wrap_left_pt=0.0):
    """A square-wrapped anchored shape whose left edge is `left_pt` from the page."""
    return f"""<w:drawing><wp:anchor distT="0" distB="0" distL="{int(wrap_left_pt*EMU_PT)}" distR="0"
        simplePos="0" relativeHeight="2" behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
      <wp:simplePos x="0" y="0"/>
      <wp:positionH relativeFrom="page"><wp:posOffset>{int(left_pt*EMU_PT)}</wp:posOffset></wp:positionH>
      <wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>
      <wp:extent cx="{int(width_pt*EMU_PT)}" cy="{int(height_pt*EMU_PT)}"/>
      <wp:effectExtent l="0" t="0" r="0" b="0"/>
      <wp:wrapSquare wrapText="bothSides"/>
      <wp:docPr id="4" name="Block"/>
      <a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
        <wps:wsp><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x="0" y="0"/>
            <a:ext cx="{int(width_pt*EMU_PT)}" cy="{int(height_pt*EMU_PT)}"/></a:xfrm>
            <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
            <a:solidFill><a:srgbClr val="DDDDDD"/></a:solidFill></wps:spPr>
          <wps:bodyPr/></wps:wsp>
      </a:graphicData></a:graphic></wp:anchor></w:drawing>"""

def document(paragraph_xml):
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
{paragraph_xml}
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
 <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""

def para(align, extra=""):
    return (f'<w:p><w:pPr><w:jc w:val="{align}"/></w:pPr>'
            f'<w:r>{extra}{inline_pic()}</w:r></w:p>')

CASES = {
    "align-only":  para("right"),
    "left-align":  para("left"),
    "band":        para("right", floating_frame(250.0, 200.0, 120.0)),
    "band-dist":   para("right", floating_frame(250.0, 200.0, 120.0, wrap_left_pt=9.0)),
}

import zipfile
for name, body in CASES.items():
    path = OUT / f"{name}.docx"
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", ROOTRELS)
        z.writestr("word/document.xml", document(body))
        z.writestr("word/_rels/document.xml.rels", DOCRELS)
        z.writestr("word/media/logo.png", png(97, 46))
    print("wrote", path)
