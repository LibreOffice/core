#!/usr/bin/env python3
"""Minimal fixtures for the one cause: shape text that overflows a fixed-height body.

Built on probes/words-extra-01/mkdocx.py, which writes a word/settings.xml part — a
hand-built DOCX without one does not get LibreOffice's OOXML compatibility defaults and
answers a different question.

The question these ask is not "does LibreOffice drop the overflowing lines" but "does it
LAY THEM OUT AND CLIP THEM", which only shows when the same PDF is read by an extractor
that honours clip paths (MuPDF) and one that does not (poppler).
"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'words-extra-01'))
from mkdocx import build, para, PGSZ            # noqa: E402
sys.path.insert(0, os.path.join(HERE, '..', 'words-extra-01'))
import importlib.util
spec = importlib.util.spec_from_file_location(
    'pto', os.path.join(HERE, '..', 'words-extra-01', 'probe-textbox-overflow.py'))

OUT = os.path.join(HERE, 'fixtures')
os.makedirs(OUT, exist_ok=True)
EMU_PT = 12700


def box_paras(n, sz=16, tag='BOXLINE'):
    return ''.join(
        f'<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/>'
        f'<w:rPr><w:sz w:val="{sz}"/></w:rPr></w:pPr>'
        f'<w:r><w:rPr><w:sz w:val="{sz}"/></w:rPr><w:t>{tag}{i:02d}</w:t></w:r></w:p>'
        for i in range(n))


def dml_box(cy_pt, nparas, cx_pt=400, autofit='<a:noAutofit/>', vertOverflow=None, sz=16):
    cy, cx = int(cy_pt * EMU_PT), int(cx_pt * EMU_PT)
    vo = f' vertOverflow="{vertOverflow}"' if vertOverflow else ''
    return f'''<w:r><mc:AlternateContent><mc:Choice Requires="wps"><w:drawing>
<wp:anchor distT="0" distB="0" distL="114300" distR="114300" simplePos="0"
 relativeHeight="251660800" behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
<wp:simplePos x="0" y="0"/>
<wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>
<wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>
<wp:extent cx="{cx}" cy="{cy}"/><wp:effectExtent l="0" t="0" r="0" b="0"/>
<wp:wrapNone/><wp:docPr id="1" name="Text Box 1"/>
<wp:cNvGraphicFramePr/>
<a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<wps:wsp><wps:cNvSpPr txBox="1"/>
<wps:spPr bwMode="auto"><a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/>
<a:ln><a:noFill/></a:ln></wps:spPr>
<wps:txbx><w:txbxContent>{box_paras(nparas, sz)}</w:txbxContent></wps:txbx>
<wps:bodyPr rot="0"{vo} vert="horz" wrap="square" lIns="91440" tIns="45720" rIns="91440"
 bIns="45720" anchor="t" anchorCtr="0" upright="1">{autofit}</wps:bodyPr>
</wps:wsp></a:graphicData></a:graphic></wp:anchor></w:drawing></mc:Choice>
<mc:Fallback><w:p/></mc:Fallback></mc:AlternateContent></w:r>'''


def vml_box(cy_pt, nparas, cx_pt=400):
    """VML v:rect/v:textbox, which is what 068 uses."""
    return (f'<w:r><w:pict><v:rect id="vr1" style="position:absolute;left:0;top:0;'
            f'width:{cx_pt}pt;height:{cy_pt}pt" filled="f" stroked="f">'
            f'<v:textbox inset="7.2pt,3.6pt,7.2pt,3.6pt"><w:txbxContent>'
            f'{box_paras(nparas)}</w:txbxContent></v:textbox></v:rect></w:pict></w:r>')


cases = {
    # the shape of all eight: fixed height, noAutofit, vertOverflow="overflow"
    'dml-h15-n6-overflow':  dml_box(15, 6, vertOverflow='overflow'),
    'dml-h15-n6-clip':      dml_box(15, 6, vertOverflow='clip'),
    'dml-h15-n6-noattr':    dml_box(15, 6),
    'dml-h15-n6-spauto':    dml_box(15, 6, autofit='<a:spAutoFit/>'),
    'dml-h15-n6-normauto':  dml_box(15, 6, autofit='<a:normAutofit/>'),
    'dml-h60-n6-overflow':  dml_box(60, 6, vertOverflow='overflow'),   # fits: control
    # a straddled line, which is where the reference keeps only the ascender glyphs
    'dml-h24-n6-overflow':  dml_box(24, 6, vertOverflow='overflow'),
    # VML, as in 068
    'vml-h15-n6':           vml_box(15, 6),
    'vml-h60-n6':           vml_box(60, 6),
}

for name, r in cases.items():
    body = f'<w:p>{r}</w:p>' + ''.join(para(f'BODY{i}') for i in range(3)) \
        + f'<w:sectPr>{PGSZ}</w:sectPr>'
    build(os.path.join(OUT, name + '.docx'), body, {})
print('built', len(cases), 'into', OUT)
