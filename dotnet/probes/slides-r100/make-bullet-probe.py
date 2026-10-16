#!/usr/bin/env python3
"""Ask 26.2.4.2 where it puts a character bullet relative to its paragraph's own
first baseline, over one variable at a time.

Outliner::StripBullet draws a SVX_NUM_CHAR_SPECIAL bullet from
ImpCalcBulletArea's box bottom less the bullet font's descent, and the box is

    Top    = nFirstLineHeight - nFirstLineTextHeight + nFirstLineTextHeight/2
             - bulletHeight/2
    Bottom = Top + bulletHeight - 1

(editeng/source/outliner/outliner.cxx:1461-1467, :892, :906-919, :951-956) while the
body's first baseline is rStartPos.Y + nFirstLineMaxAscent.  So the offset between the
two reads nFirstLineHeight and nFirstLineTextHeight and nothing else does, and those two
are what round 99 could not pin.  This deck varies the stated size and the stated line
spacing, which are the two inputs to them.

One box per case, anchor top, a:noAutofit, no insets; the bullet is U+2022 at 100% of
the run.

    make-bullet-probe.py <out.pptx>
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

def shape(idx, name, x, y, cx, cy, body):
    return (f'<p:sp><p:nvSpPr><p:cNvPr id="{idx}" name="{name}"/>'
            f'<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
            f'<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
            f'<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0"'
            f' anchor="t"><a:noAutofit/></a:bodyPr><a:lstStyle/>{body}</p:txBody></p:sp>')

CASES = [(sz, spc) for sz in (1200, 1800, 2400, 3600) for spc in (0, 70, 80, 90, 100, 120)]

def slide(size, spc):
    return (HEAD
            + shape(2, 'Spacer', 200000, 60000, 2000000, 300000,
                    para('spacer', 1200, 0, bullet=False))
            + shape(3, 'BUL', int(40 * EMU_PT), int(80 * EMU_PT),
                    int(400 * EMU_PT), int(400 * EMU_PT),
                    para('Hxy', size, spc) + para('Hxy', size, spc))
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
        for i, (sz, spc) in enumerate(CASES):
            z.writestr(f'ppt/slides/slide{i+1}.xml', slide(sz, spc))
            z.writestr(f'ppt/slides/_rels/slide{i+1}.xml.rels', SLIDE_RELS)
    print(f'{a.out}: {len(CASES)} slides {CASES}')
