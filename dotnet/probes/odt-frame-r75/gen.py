#!/usr/bin/env python3
"""Authored flat-ODF probes for the auto-height (SwFrameSize::Minimum) frame rule.

Each probe is one paragraph-anchored draw:frame with svg:width and no svg:height,
whose draw:text-box states fo:min-height.  The frame's automatic graphic style names
a PARENT style, which is what makes LibreOffice import it as a Writer text frame
rather than as a drawing shape: XMLTextFrameContext's ctor sets
m_HasAutomaticStyleWithoutParentStyle for a parentless automatic style
(xmloff/source/text/XMLTextFrameContext.cxx:1374-1394, "#i51726#") and
createFastChildContext then routes such a draw:frame to
XMLShapeImportHelper::CreateFrameChildContext (:1500-1507).  A shape autofits to its
text and ignores fo:min-height entirely; only the text frame is SwFrameSize::Minimum.
"""
import os, sys

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/odtframe-probes"
os.makedirs(OUT, exist_ok=True)

HEAD = """<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 xmlns:loext="urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
 <office:styles>
  <style:style style:name="Frame" style:family="graphic">
   <style:graphic-properties text:anchor-type="paragraph" svg:x="0in" svg:y="0in"
    fo:margin-left="0in" fo:margin-right="0in" fo:margin-top="0in" fo:margin-bottom="0in"
    style:wrap="parallel" style:vertical-pos="top" style:vertical-rel="paragraph-content"
    style:horizontal-pos="center" style:horizontal-rel="paragraph-content"
    draw:fill="none" fo:padding="0in" fo:border="0.06pt solid #000000"/>
  </style:style>
 </office:styles>
 <office:automatic-styles>
  <style:style style:name="PM1" style:family="page-layout">
   <style:page-layout-properties fo:page-width="8.5in" fo:page-height="11in"
    fo:margin-top="1in" fo:margin-bottom="1in" fo:margin-left="1in" fo:margin-right="1in"/>
  </style:style>
  <style:style style:name="P1" style:family="paragraph">
   <style:paragraph-properties fo:margin-top="0in" fo:margin-bottom="{after}"/>
   <style:text-properties style:font-name="Liberation Serif" fo:font-size="12pt"/>
  </style:style>
  <style:style style:name="PA" style:family="paragraph">
   <style:paragraph-properties fo:margin-top="0in" fo:margin-bottom="0in"/>
   <style:text-properties style:font-name="Liberation Serif" fo:font-size="12pt"/>
  </style:style>
  <style:style style:name="fr1" style:family="graphic" style:parent-style-name="Frame">
   <style:graphic-properties fo:margin-left="0in" fo:margin-right="0in"
    fo:margin-top="0in" fo:margin-bottom="0in"
    style:vertical-pos="from-top" style:vertical-rel="paragraph"
    style:horizontal-pos="from-left" style:horizontal-rel="page"
    style:wrap="{wrap}" draw:fill="none"
    fo:padding="{pad}" fo:border="{border}"/>
  </style:style>
 </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="Standard" style:page-layout-name="PM1"/>
 </office:master-styles>
 <office:body><office:text>
"""

TAIL = " </office:text></office:body></office:document>\n"


def probe(name, paras=2, minh="0in", pad="0in", border="0.06pt solid #000000",
          after="0in", wrap="none", width="4in", split="true", body=6, size="12pt"):
    inner = "".join(
        f'<text:p text:style-name="P1">Line {i + 1} of the frame.</text:p>'
        for i in range(paras))
    frame = (
        f'<draw:frame draw:style-name="fr1" draw:name="F1" text:anchor-type="paragraph"'
        f' svg:x="1in" svg:y="0in" svg:width="{width}" draw:z-index="0"'
        f' loext:may-break-between-pages="{split}">'
        f'<draw:text-box fo:min-height="{minh}">{inner}</draw:text-box></draw:frame>')
    body_paras = "".join(
        f'<text:p text:style-name="PA">Body paragraph {i + 1}.</text:p>'
        for i in range(body))
    xml = (HEAD.format(after=after, pad=pad, border=border, wrap=wrap)
           + f'<text:p text:style-name="PA">{frame}Anchor.</text:p>'
           + body_paras + TAIL)
    path = os.path.join(OUT, name + ".fodt")
    with open(path, "w", encoding="utf-8") as f:
        f.write(xml)
    return path


CASES = [
    ("a-2para",                dict(paras=2)),
    ("b-4para",                dict(paras=4)),
    ("c-2para-after12pt",      dict(paras=2, after="12pt")),
    ("d-4para-after12pt",      dict(paras=4, after="12pt")),
    ("e-2para-pad10pt",        dict(paras=2, pad="10pt")),
    ("f-2para-border3pt",      dict(paras=2, border="3pt solid #000000")),
    ("g-2para-pad10-bord3",    dict(paras=2, pad="10pt", border="3pt solid #000000")),
    ("h-2para-minh3in",        dict(paras=2, minh="3in")),
    ("i-2para-minh3in-pad10",  dict(paras=2, minh="3in", pad="10pt")),
    ("j-1para-minh0",          dict(paras=1)),
    ("k-0para-minh0",          dict(paras=0)),
    ("l-2para-noborder",       dict(paras=2, border="none")),
    ("m-2para-minh0.2in",      dict(paras=2, minh="0.2in")),
    ("n-60para-split",         dict(paras=60, split="true", body=3)),
    ("o-60para-nosplit",       dict(paras=60, split="false", body=3)),
    ("p-2para-8pt",            dict(paras=2, size="8pt")),
]

for name, kw in CASES:
    kw.pop("size", None)
    print(probe(name, **kw))
