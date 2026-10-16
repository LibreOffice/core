#!/usr/bin/env python3
"""Ask the INSTALLED 26.2.4.2 whether a hard line break's own character height
sizes the line it ends -- and, on the same deck, whether a trailing blank's does.

editeng/source/editeng/impedit3.cxx:1498-1516 walks every portion of a line from
GetStartPortion() to GetEndPortion() and takes the largest ascent and descent,
skipping ONLY `PortionKind::LINEBREAK`, which EE_FEATURE_LINEBR sets (:1088-1099).
So a trailing blank sizes its line and a hard break does not.  That is read out of
a 27.2.0.0.alpha0+ checkout; this deck measures the claim at the binary.

Three shapes per slide, each `a:noAutofit`, `anchor="t"`, no insets:

  BREAK   "AAA" 18pt, <a:br sz=S>, "BBB" 18pt   -- one paragraph, two lines
  BLANK   "AAA " with the trailing space at S,  -- two paragraphs, one line each
          then "BBB" 18pt
  RUN     "AAA" 18pt + "X" at S, then "BBB"     -- control: S is on line 1's text

The pitch between the two baselines is read out of the reference's own PDF.
BREAK must not move with S; BLANK and RUN must.

    make-break-probe.py <out.pptx> [--sizes 1800,2800,3600,5400,7200]
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

def rpr(size):
    return f'<a:rPr lang="en-GB" sz="{size}"><a:latin typeface="Liberation Sans"/></a:rPr>'

def run(text, size):
    return f'<a:r>{rpr(size)}<a:t>{text}</a:t></a:r>'

def br(size):
    return f'<a:br>{rpr(size)}</a:br>'

def shape(idx, name, x, y, cx, cy, body):
    return (f'<p:sp><p:nvSpPr><p:cNvPr id="{idx}" name="{name}"/>'
            f'<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
            f'<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
            f'<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0"'
            f' anchor="t"><a:noAutofit/></a:bodyPr><a:lstStyle/>{body}</p:txBody></p:sp>')

BASE = 1800

def slide(size):
    brk   = f'<a:p>{run("AAA", BASE)}{br(size)}{run("BBB", BASE)}</a:p>'
    blank = (f'<a:p>{run("AAA", BASE)}{run(" ", size)}</a:p>'
             f'<a:p>{run("BBB", BASE)}</a:p>')
    ctrl  = (f'<a:p>{run("AAA", BASE)}{run("X", size)}</a:p>'
             f'<a:p>{run("BBB", BASE)}</a:p>')
    # an empty line whose only portion is the break, between two 18 pt lines
    twice = f'<a:p>{run("AAA", BASE)}{br(size)}{br(size)}{run("BBB", BASE)}</a:p>'
    # a break that ends the paragraph: EditEngine appends an empty line after it
    tail  = (f'<a:p>{run("AAA", BASE)}{br(size)}</a:p>'
             f'<a:p>{run("BBB", BASE)}</a:p>')
    parts = [shape(2, 'Spacer', 200000, 60000, 2000000, 300000,
                   f'<a:p>{run("spacer", 1200)}</a:p>'),
             shape(3, 'BREAK', int(20 * EMU_PT), int(60 * EMU_PT),
                   int(200 * EMU_PT), int(200 * EMU_PT), brk),
             shape(4, 'BLANK', int(260 * EMU_PT), int(60 * EMU_PT),
                   int(200 * EMU_PT), int(200 * EMU_PT), blank),
             shape(5, 'RUN', int(500 * EMU_PT), int(60 * EMU_PT),
                   int(200 * EMU_PT), int(200 * EMU_PT), ctrl),
             shape(6, 'TWICE', int(20 * EMU_PT), int(300 * EMU_PT),
                   int(200 * EMU_PT), int(220 * EMU_PT), twice),
             shape(7, 'TAIL', int(260 * EMU_PT), int(300 * EMU_PT),
                   int(200 * EMU_PT), int(220 * EMU_PT), tail)]
    return HEAD + ''.join(parts) + TAIL

if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('out')
    ap.add_argument('--sizes', default='1800,2400,2800,3600,5400,7200')
    a = ap.parse_args()
    sizes = [int(s) for s in a.sizes.split(',')]

    ct = [CT_HEAD]; ids = []; rels = []
    for i in range(len(sizes)):
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
        for i, s in enumerate(sizes):
            z.writestr(f'ppt/slides/slide{i+1}.xml', slide(s))
            z.writestr(f'ppt/slides/_rels/slide{i+1}.xml.rels', SLIDE_RELS)
    print(f'{a.out}: {len(sizes)} slides, break/blank sizes {sizes}')
