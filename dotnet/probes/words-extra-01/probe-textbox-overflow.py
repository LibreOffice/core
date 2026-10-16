#!/usr/bin/env python3
"""Probe: how does LibreOffice treat a fixed-height wps text box whose text overflows?"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx import build, para, PGSZ

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'probesbox')
os.makedirs(OUT, exist_ok=True)

EMU_PT = 12700


def box_paras(n, sz=16):
    """n paragraphs of distinguishable text at half-point size sz."""
    out = []
    for i in range(n):
        out.append(
            f'<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" '
            f'w:lineRule="auto"/><w:rPr><w:sz w:val="{sz}"/></w:rPr></w:pPr>'
            f'<w:r><w:rPr><w:sz w:val="{sz}"/></w:rPr>'
            f'<w:t>BOXLINE{i:02d}</w:t></w:r></w:p>')
    return ''.join(out)


def textbox(cy_pt, nparas, cx_pt=400, autofit='<a:noAutofit/>', tIns=45720, bIns=45720,
            vertOverflow=None, sz=16):
    cy = int(cy_pt * EMU_PT)
    cx = int(cx_pt * EMU_PT)
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
<wps:bodyPr rot="0"{vo} vert="horz" wrap="square" lIns="91440" tIns="{tIns}" rIns="91440"
 bIns="{bIns}" anchor="t" anchorCtr="0" upright="1">{autofit}</wps:bodyPr>
</wps:wsp></a:graphicData></a:graphic></wp:anchor></w:drawing></mc:Choice>
<mc:Fallback><w:p/></mc:Fallback></mc:AlternateContent></w:r>'''


cases = {}
# Sweep box height with 6 paragraphs of 8 pt text (line ~ 9.7 pt).
for h in [8, 10, 12, 15, 20, 25, 30, 40, 50, 60, 100]:
    cases[f'box-h{h}-n6'] = textbox(h, 6)
# Anchor bottom / centre with a tall overflow
cases['box-h20-n6-anchorb'] = textbox(20, 6).replace('anchor="t"', 'anchor="b"')
cases['box-h20-n6-anchorctr'] = textbox(20, 6).replace('anchor="t"', 'anchor="ctr"')
# spAutoFit (box grows to text)
cases['box-h15-n6-spauto'] = textbox(15, 6, autofit='<a:spAutoFit/>')
# normAutofit (text shrinks)
cases['box-h15-n6-normauto'] = textbox(15, 6, autofit='<a:normAutofit/>')
# vertOverflow=overflow / clip
cases['box-h15-n6-ovf'] = textbox(15, 6, vertOverflow='overflow')
cases['box-h15-n6-clip'] = textbox(15, 6, vertOverflow='clip')
# a single long wrapping paragraph instead of many
LONGP = ('<w:p><w:pPr><w:spacing w:before="0" w:after="0"/><w:rPr><w:sz w:val="16"/></w:rPr></w:pPr>'
         '<w:r><w:rPr><w:sz w:val="16"/></w:rPr><w:t>'
         + ' '.join(f'WORD{i:02d}' for i in range(60)) + '</w:t></w:r></w:p>')
tb = textbox(15, 1, cx_pt=100)
cases['box-h15-wrap'] = tb.replace(box_paras(1, 16), LONGP)
cases['box-h40-wrap'] = textbox(40, 1, cx_pt=100).replace(box_paras(1, 16), LONGP)

for name, r in cases.items():
    body = f'<w:p>{r}</w:p>' + ''.join(para(f'BODY{i}') for i in range(3)) \
        + f'<w:sectPr>{PGSZ}</w:sectPr>'
    build(os.path.join(OUT, name + '.docx'), body, {})

print('built', len(cases))
