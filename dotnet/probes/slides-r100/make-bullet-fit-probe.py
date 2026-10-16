#!/usr/bin/env python3
"""The second half of make-bullet-probe.py: the same bullet offset, with the box
small enough that the autofit search runs and `maScalingParameters.fSpacingY` is
below one.

That is the only input to EditEngine's two line-spacing branches
(`editeng/source/editeng/impedit3.cxx`:1553-1600) that make-bullet-probe.py does not
vary, and it is what the corpus witnesses carry: on `71393_pp7.ppt` page 13 the stated
90% and the fit's 0.800 multiply to the 0.72 that our own H and A already follow.

One box per slide, `a:normAutofit`, six 24 pt bulleted paragraphs, box height swept so
the search lands on a different row of `constScaleLevels` each time; the stated line
spacing is a second variable.

    make-bullet-fit-probe.py <out.pptx>
"""
import argparse, zipfile, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from _parts import (CT_HEAD, ROOT_RELS, SLIDE_RELS, LAYOUT, LAYOUT_RELS,
                    MASTER, MASTER_RELS, THEME, EMU_PT, SLIDE_W, SLIDE_H)

HEAD = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"'
        ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
        ' xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">'
        '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/>'
        '<p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>')
TAIL = '</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>'

def para(text, size, spc, bullet=True):
    ln = f'<a:lnSpc><a:spcPct val="{spc * 1000}"/></a:lnSpc>' if spc else ''
    bu = ('<a:buFont typeface="StarSymbol"/><a:buChar char="•"/>'
          if bullet else '<a:buNone/>')
    return (f'<a:p><a:pPr marL="457200" indent="-457200">{ln}{bu}</a:pPr>'
            f'<a:r><a:rPr lang="en-GB" sz="{size}">'
            f'<a:latin typeface="Liberation Sans"/></a:rPr><a:t>{text}</a:t></a:r></a:p>')

def shape(idx, name, x, y, cx, cy, body, fit):
    return (f'<p:sp><p:nvSpPr><p:cNvPr id="{idx}" name="{name}"/>'
            f'<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
            f'<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
            f'<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0"'
            f' anchor="t">{fit}</a:bodyPr><a:lstStyle/>{body}</p:txBody></p:sp>')

CASES = [(h, spc) for h in (60, 90, 120, 160, 200, 260, 340, 420)
                  for spc in (0, 90)]

def slide(height, spc):
    body = ''.join(para(f'Hxy {i}', 2400, spc) for i in range(6))
    return (HEAD
            + shape(2, 'Spacer', 200000, 60000, 2000000, 300000,
                    para('spacer', 1200, 0, bullet=False), '<a:noAutofit/>')
            + shape(3, 'BUL', int(40 * EMU_PT), int(80 * EMU_PT),
                    int(400 * EMU_PT), int(height * EMU_PT), body, '<a:normAutofit/>')
            + TAIL)

if __name__ == '__main__':
    ap = argparse.ArgumentParser(); ap.add_argument('out'); a = ap.parse_args()
    ct = [CT_HEAD]; ids = []; rels = []
    for i in range(len(CASES)):
        n = i + 1
        ct.append(f'<Override PartName="/ppt/slides/slide{n}.xml" ContentType='
                  f'"application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>')
        ids.append(f'<p:sldId id="{255 + n}" r:id="rId{100 + n}"/>')
        rels.append(f'<Relationship Id="rId{100 + n}" Type="http://schemas.openxmlformats.org'
                    f'/officeDocument/2006/relationships/slide" Target="slides/slide{n}.xml"/>')
    ct.append('</Types>')
    pres = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<p:presentation xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"'
            ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
            ' xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">'
            '<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/></p:sldMasterIdLst>'
            f'<p:sldIdLst>{"".join(ids)}</p:sldIdLst>'
            f'<p:sldSz cx="{SLIDE_W}" cy="{SLIDE_H}"/><p:notesSz cx="6858000" cy="9144000"/>'
            '</p:presentation>')
    pres_rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                 '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>'
                 f'{"".join(rels)}</Relationships>')
    with zipfile.ZipFile(a.out, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ''.join(ct))
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('ppt/presentation.xml', pres)
        z.writestr('ppt/_rels/presentation.xml.rels', pres_rels)
        z.writestr('ppt/slideLayouts/slideLayout1.xml', LAYOUT)
        z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels', LAYOUT_RELS)
        z.writestr('ppt/slideMasters/slideMaster1.xml', MASTER)
        z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels', MASTER_RELS)
        z.writestr('ppt/theme/theme1.xml', THEME)
        for i, (h, spc) in enumerate(CASES):
            z.writestr(f'ppt/slides/slide{i+1}.xml', slide(h, spc))
            z.writestr(f'ppt/slides/_rels/slide{i+1}.xml.rels', SLIDE_RELS)
    print(f'{a.out}: {len(CASES)} slides {CASES}')
