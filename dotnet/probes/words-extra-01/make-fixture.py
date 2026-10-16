#!/usr/bin/env python3
"""Authors `tests/corpus/features/textbox-overflow.docx`.

One page holding three DrawingML text boxes, each with the same six numbered
paragraphs of 8 pt text and differing only in how tall the box is and whether it
autofits:

  BOXA  30 pt, `a:noAutofit`   — three lines fit
  BOXB   8 pt, `a:noAutofit`   — no line fits, so exactly one is drawn anyway
  BOXC  15 pt, `a:spAutoFit`   — the box grows, so all six are drawn

The heights come straight from the sweep in `probe-textbox-sweep.py`, which is
what fixes the expected line counts: with the ECMA default 45720 EMU top and
bottom insets a 30 pt box holds three lines of this text and a 28 pt box holds
three but a 26 pt box holds two, so 30 sits well inside its bracket and is not a
measurement balanced on a rounding edge.

Run from anywhere; it writes into the corpus by absolute path.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from mkdocx import build, para, PGSZ  # noqa: E402

TARGET = os.path.normpath(os.path.join(HERE, '..', '..', 'tests', 'corpus', 'features',
                                       'textbox-overflow.docx'))
EMU_PT = 12700


def lines(tag, n=6):
    """n single-spaced 8 pt paragraphs, each naming itself."""
    return ''.join(
        f'<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/>'
        f'<w:rPr><w:sz w:val="16"/></w:rPr></w:pPr>'
        f'<w:r><w:rPr><w:sz w:val="16"/></w:rPr><w:t>{tag}{i}</w:t></w:r></w:p>'
        for i in range(n))


def box(tag, cy_pt, autofit, y_pt):
    cy = int(cy_pt * EMU_PT)
    cx = int(300 * EMU_PT)
    return f'''<w:r><mc:AlternateContent><mc:Choice Requires="wps"><w:drawing>
<wp:anchor distT="0" distB="0" distL="114300" distR="114300" simplePos="0"
 relativeHeight="251660800" behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
<wp:simplePos x="0" y="0"/>
<wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>
<wp:positionV relativeFrom="paragraph"><wp:posOffset>{int(y_pt * EMU_PT)}</wp:posOffset></wp:positionV>
<wp:extent cx="{cx}" cy="{cy}"/><wp:effectExtent l="0" t="0" r="0" b="0"/>
<wp:wrapNone/><wp:docPr id="{ord(tag[-1])}" name="Text Box {tag}"/>
<wp:cNvGraphicFramePr/>
<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<wps:wsp><wps:cNvSpPr txBox="1"/>
<wps:spPr bwMode="auto"><a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/>
<a:ln><a:noFill/></a:ln></wps:spPr>
<wps:txbx><w:txbxContent>{lines(tag)}</w:txbxContent></wps:txbx>
<wps:bodyPr rot="0" vert="horz" wrap="square" lIns="91440" tIns="45720" rIns="91440"
 bIns="45720" anchor="t" anchorCtr="0" upright="1">{autofit}</wps:bodyPr>
</wps:wsp></a:graphicData></a:graphic></wp:anchor></w:drawing></mc:Choice>
<mc:Fallback><w:p/></mc:Fallback></mc:AlternateContent></w:r>'''


body = (
    '<w:p>'
    + box('BOXA', 30, '<a:noAutofit/>', 0)
    + box('BOXB', 8, '<a:noAutofit/>', 120)
    + box('BOXC', 15, '<a:spAutoFit/>', 240)
    + '</w:p>'
    + para('BODYTEXT')
    + f'<w:sectPr>{PGSZ}</w:sectPr>')

build(TARGET, body, {})
print('wrote', TARGET, os.path.getsize(TARGET), 'bytes')
