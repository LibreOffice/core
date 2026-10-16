#!/usr/bin/env python3
"""Writes `tests/corpus/features/slide-text-vertical.pptx`, the fixture behind
`SlideVerticalTextTests`.

One slide, three otherwise identical 230 x 160 pt boxes at `vert="horz"`, `"vert"` and
`"vert270"`, each carrying one 14 pt word and **asymmetric insets** of 10 / 20 / 30 / 40 pt.
The asymmetry is the point: with the DrawingML defaults, or with any symmetric quadruple, a
reader that forgets to rotate the insets is indistinguishable from one that remembers.

One word rather than a paragraph, so the assertions are about a single run origin and cannot be
confounded by where a line breaks.

    make-vert-fixture.py           # writes into the repository
    make-vert-fixture.py <outdir>
"""
import os, sys, zipfile

_SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "make-oblique-probe.py")
_NS = {"__name__": "_scaffold"}
exec(compile(open(_SRC).read(), _SRC, "exec"), _NS)  # noqa: S102
CT, ROOT_RELS, THEME = _NS["CT"], _NS["ROOT_RELS"], _NS["THEME"]
MASTER, MASTER_RELS = _NS["MASTER"], _NS["MASTER_RELS"]
LAYOUT, LAYOUT_RELS, SLIDE_RELS = _NS["LAYOUT"], _NS["LAYOUT_RELS"], _NS["SLIDE_RELS"]
presentation, pres_rels = _NS["presentation"], _NS["pres_rels"]

EMU = 12700
VERTS = ["horz", "vert", "vert270"]
INSETS = (10, 20, 30, 40)          # lIns, tIns, rIns, bIns, in points
BOX = (200000, 300000, 230 * EMU, 160 * EMU)
STEP = 3000000


def box(shape_id, x, y, w, h, vert):
    l, t, r, b = (n * EMU for n in INSETS)
    return f"""<p:sp><p:nvSpPr><p:cNvPr id="{shape_id}" name="{vert}"/>
<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>
<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{w}" cy="{h}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>
<p:txBody><a:bodyPr vert="{vert}" anchor="t" lIns="{l}" tIns="{t}" rIns="{r}" bIns="{b}"
 wrap="square"><a:noAutofit/></a:bodyPr>
<a:lstStyle/><a:p><a:pPr algn="l"><a:lnSpc><a:spcPct val="100000"/></a:lnSpc></a:pPr>
<a:r><a:rPr lang="en-US" sz="1400" b="0" i="0"><a:solidFill><a:srgbClr val="000000"/></a:solidFill>
<a:latin typeface="Liberation Sans"/></a:rPr><a:t>Turn</a:t></a:r></a:p></p:txBody></p:sp>"""


def slide():
    x, y, w, h = BOX
    shapes = "".join(
        box(2 + i, x + i * STEP, y, w, h, v) for i, v in enumerate(VERTS))
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
<p:grpSpPr/>{shapes}</p:spTree></p:cSld></p:sld>"""


if __name__ == "__main__":
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "..", "..", "tests", "corpus", "features")
    out = os.path.abspath(out)
    os.makedirs(out, exist_ok=True)
    path = os.path.join(out, "slide-text-vertical.pptx")
    over = ('<Override PartName="/ppt/slides/slide1.xml" ContentType='
            '"application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>')
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT.format(slides=over))
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("ppt/presentation.xml", presentation(1))
        z.writestr("ppt/_rels/presentation.xml.rels", pres_rels(1))
        z.writestr("ppt/theme/theme1.xml", THEME)
        z.writestr("ppt/slideMasters/slideMaster1.xml", MASTER)
        z.writestr("ppt/slideMasters/_rels/slideMaster1.xml.rels", MASTER_RELS)
        z.writestr("ppt/slideLayouts/slideLayout1.xml", LAYOUT)
        z.writestr("ppt/slideLayouts/_rels/slideLayout1.xml.rels", LAYOUT_RELS)
        z.writestr("ppt/slides/slide1.xml", slide())
        z.writestr("ppt/slides/_rels/slide1.xml.rels", SLIDE_RELS)
    print(path)
