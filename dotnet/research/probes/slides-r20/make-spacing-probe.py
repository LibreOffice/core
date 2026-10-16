#!/usr/bin/env python3
"""Build a probe deck that separates two readings of the autofit spacing scale.

The shrink search may fall back on a line-spacing scale of 90 or 80 per cent. The open
question is whether that scale also reaches a *paragraph's* own space-before, or only the
line heights.

Each box here holds four paragraphs of two lines each, the second line forced by a hard
break so the line count cannot change with the font size. So one box gives both
measurements at once:

    within a paragraph   baseline pitch          = the scaled line height
    between paragraphs   baseline pitch + extra  = extra is the space-before, scaled or not

Box heights sweep so that some boxes land on a spacing reduction and some do not.
"""
import os
import shutil
import subprocess
import sys
import zipfile

SRC = os.path.dirname(os.path.abspath(__file__)) + "/grid"
OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/spacing-probe.pptx"

EMU_PT = 12700
HEIGHTS = [150, 160, 170, 180, 190, 200, 205, 210, 215, 220, 225, 230]
PER_SLIDE = 3
SPC_BEF = 1200          # hundredths of a point: 12 pt
SZ = 2000               # 20 pt

SLIDE_NS = ('<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
            'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
            'xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">')


def shape(sid, name, x_emu, y_emu, cx, cy, autofit, paras):
    body = "".join(paras)
    fit = "<a:normAutofit/>" if autofit else "<a:noAutofit/>"
    return (
        f'<p:sp><p:nvSpPr><p:cNvPr id="{sid}" name="{name}"/><p:cNvSpPr txBox="1"/>'
        f'<p:nvPr/></p:nvSpPr><p:spPr><a:xfrm><a:off x="{x_emu}" y="{y_emu}"/>'
        f'<a:ext cx="{cx}" cy="{cy}"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
        f'<a:noFill/></p:spPr><p:txBody>'
        f'<a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0" anchor="t">{fit}</a:bodyPr>'
        f'<a:lstStyle/>{body}</p:txBody></p:sp>')


def para(words, spc_bef):
    rpr = f'<a:rPr lang="en-GB" sz="{SZ}"><a:latin typeface="Liberation Sans"/></a:rPr>'
    brk = f'<a:br>{rpr}</a:br>'
    runs = f'<a:r>{rpr}<a:t>{words[0]}</a:t></a:r>{brk}<a:r>{rpr}<a:t>{words[1]}</a:t></a:r>'
    pre = f'<a:spcBef><a:spcPts val="{spc_bef}"/></a:spcBef>' if spc_bef else ''
    return f'<a:p><a:pPr algn="l">{pre}</a:pPr>{runs}</a:p>'


WORDS = [("Alpha", "Bravo"), ("Charlie", "Delta"), ("Echo", "Foxtrot"), ("Golf", "Hotel")]


def build():
    if os.path.exists(OUT):
        os.remove(OUT)
    slides = []
    for start in range(0, len(HEIGHTS), PER_SLIDE):
        chunk = HEIGHTS[start:start + PER_SLIDE]
        shapes = []
        # A sacrificial first shape: LibreOffice's shared draw outliner formats the first
        # text object on a page before SetFixedCellHeight takes hold, so nothing under test
        # may be first.
        shapes.append(shape(2, "warmup", 100 * EMU_PT, 10 * EMU_PT,
                            200 * EMU_PT, 30 * EMU_PT, False,
                            [para(("Warm", "Up"), 0)]))
        for i, h in enumerate(chunk):
            x = (10 + i * 230) * EMU_PT
            shapes.append(shape(3 + i, f"h{h}", x, 60 * EMU_PT,
                                220 * EMU_PT, h * EMU_PT, True,
                                [para(w, SPC_BEF) for w in WORDS]))
        slides.append(
            SLIDE_NS +
            '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/>'
            '<p:nvPr/></p:nvGrpSpPr><p:grpSpPr><a:xfrm><a:off x="0" y="0"/>'
            '<a:ext cx="0" cy="0"/><a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm>'
            '</p:grpSpPr>' + "".join(shapes) +
            '</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>')

    n = len(slides)
    with zipfile.ZipFile(OUT, "w", zipfile.ZIP_DEFLATED) as z:
        for root, _, files in os.walk(SRC):
            for f in files:
                full = os.path.join(root, f)
                rel = os.path.relpath(full, SRC)
                if rel.startswith("ppt/slides/") or rel == "[Content_Types].xml" \
                        or rel == "ppt/presentation.xml" or rel == "ppt/_rels/presentation.xml.rels":
                    continue
                z.write(full, rel)
        ct = open(os.path.join(SRC, "[Content_Types].xml")).read()
        extra = "".join(
            f'<Override PartName="/ppt/slides/slide{i+1}.xml" ContentType="application/'
            'vnd.openxmlformats-officedocument.presentationml.slide+xml"/>' for i in range(n))
        ct = ct.replace(
            '<Override PartName="/ppt/slides/slide1.xml" ContentType="application/'
            'vnd.openxmlformats-officedocument.presentationml.slide+xml"/>', extra)
        z.writestr("[Content_Types].xml", ct)

        ids = "".join(f'<p:sldId id="{256+i}" r:id="rId{10+i}"/>' for i in range(n))
        pres = open(os.path.join(SRC, "ppt/presentation.xml")).read()
        pres = pres.replace('<p:sldId id="256" r:id="rId2"/>', ids)
        z.writestr("ppt/presentation.xml", pres)

        rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                'relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>'
                '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                'relationships/theme" Target="theme/theme1.xml"/>' +
                "".join(f'<Relationship Id="rId{10+i}" Type="http://schemas.openxmlformats.org/'
                        f'officeDocument/2006/relationships/slide" Target="slides/slide{i+1}.xml"/>'
                        for i in range(n)) + '</Relationships>')
        z.writestr("ppt/_rels/presentation.xml.rels", rels)

        srel = open(os.path.join(SRC, "ppt/slides/_rels/slide1.xml.rels")).read()
        for i, s in enumerate(slides):
            z.writestr(f"ppt/slides/slide{i+1}.xml",
                       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' + s)
            z.writestr(f"ppt/slides/_rels/slide{i+1}.xml.rels", srel)
    print(OUT, n, "slides", len(HEIGHTS), "boxes")


build()
