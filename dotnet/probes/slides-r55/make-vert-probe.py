#!/usr/bin/env python3
"""A known-answer deck for `a:bodyPr/@vert`, authored by hand.

One slide per `ST_TextVerticalType` value, each carrying three identical 320 x 160 pt boxes at
`anchor` t / ctr / b, with text long enough to break.  What has to be read out of the reference:

  * which way the text turns, and whether it turns at all (`wordArtVert` is STACKED in
    LibreOffice, not a turn, and `mongolianVert` is documented as not implemented for shape text);
  * whether the *layout box transposes* -- lines breaking at the shape's height rather than its
    width -- which is the difference between `Turned()` and `TextAreaTurn()` in
    `PptxSlideLayout.cs` and decides where every line lands;
  * where each anchor puts the block once it is turned, because `textbodypropertiescontext.cxx`
    swaps the horizontal and vertical adjusts for `eaVert` and `mongolianVert` and leaves them
    alone for `vert` and `vert270`.

Authored rather than round-tripped, for the reason round 54 recorded: a fixture built through
`soffice --convert-to` inherits the exporter's defaults and can make two rival rules
indistinguishable.

    make-vert-probe.py <outdir>
"""
import os, sys, zipfile

VERTS = ["horz", "vert", "vert270", "eaVert", "mongolianVert", "wordArtVert"]
ANCHORS = ["t", "ctr", "b"]
TEXT = "Alpha bravo charlie delta echo foxtrot golf hotel india"
SIZE = 1400

# The scaffolding is identical to make-oblique-probe.py's; imported by exec rather than copied
# so the two decks cannot drift apart in a way that makes their results incomparable.
_SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "make-oblique-probe.py")
_NS = {"__name__": "_scaffold"}
exec(compile(open(_SRC).read(), _SRC, "exec"), _NS)  # noqa: S102
CT, ROOT_RELS, THEME = _NS["CT"], _NS["ROOT_RELS"], _NS["THEME"]
MASTER, MASTER_RELS = _NS["MASTER"], _NS["MASTER_RELS"]
LAYOUT, LAYOUT_RELS, SLIDE_RELS = _NS["LAYOUT"], _NS["LAYOUT_RELS"], _NS["SLIDE_RELS"]
presentation, pres_rels = _NS["presentation"], _NS["pres_rels"]

EMU_PT = 12700


def box(shape_id, x, y, w, h, vert, anchor):
    return f"""<p:sp><p:nvSpPr><p:cNvPr id="{shape_id}" name="s{shape_id}"/>
<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>
<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{w}" cy="{h}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/>
<a:ln w="6350"><a:solidFill><a:srgbClr val="C00000"/></a:solidFill></a:ln></p:spPr>
<p:txBody><a:bodyPr vert="{vert}" anchor="{anchor}" lIns="0" tIns="0" rIns="0" bIns="0"
 wrap="square"><a:noAutofit/></a:bodyPr>
<a:lstStyle/><a:p><a:pPr algn="l"><a:lnSpc><a:spcPct val="100000"/></a:lnSpc></a:pPr>
<a:r><a:rPr lang="en-US" sz="{SIZE}" b="0" i="0" dirty="0">
<a:solidFill><a:srgbClr val="000000"/></a:solidFill>
<a:latin typeface="Liberation Sans"/></a:rPr><a:t>{TEXT}</a:t></a:r>
<a:endParaRPr lang="en-US" sz="{SIZE}"/></a:p></p:txBody></p:sp>"""


def slide(vert):
    shapes = []
    sid = 2
    for i, anchor in enumerate(ANCHORS):
        shapes.append(box(sid, 200000 + i * 3000000, 300000,
                          230 * EMU_PT, 160 * EMU_PT, vert, anchor))
        sid += 1
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
<p:grpSpPr/>{''.join(shapes)}</p:spTree></p:cSld></p:sld>"""


if __name__ == "__main__":
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    path = os.path.join(out, "vert-probe.pptx")
    n = len(VERTS)
    over = "".join(
        f'<Override PartName="/ppt/slides/slide{i + 1}.xml" '
        f'ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>'
        for i in range(n))
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT.format(slides=over))
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("ppt/presentation.xml", presentation(n))
        z.writestr("ppt/_rels/presentation.xml.rels", pres_rels(n))
        z.writestr("ppt/theme/theme1.xml", THEME)
        z.writestr("ppt/slideMasters/slideMaster1.xml", MASTER)
        z.writestr("ppt/slideMasters/_rels/slideMaster1.xml.rels", MASTER_RELS)
        z.writestr("ppt/slideLayouts/slideLayout1.xml", LAYOUT)
        z.writestr("ppt/slideLayouts/_rels/slideLayout1.xml.rels", LAYOUT_RELS)
        for i, v in enumerate(VERTS):
            z.writestr(f"ppt/slides/slide{i + 1}.xml", slide(v))
            z.writestr(f"ppt/slides/_rels/slide{i + 1}.xml.rels", SLIDE_RELS)
    print(path)
    for i, v in enumerate(VERTS, 1):
        print(f"  slide {i}: vert={v}  (boxes: anchor {', '.join(ANCHORS)})")
