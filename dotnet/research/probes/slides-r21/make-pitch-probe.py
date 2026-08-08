#!/usr/bin/env python3
"""A probe that measures the *natural* line pitch of an unfitted slide text box.

No autofit anywhere: every box is `a:noAutofit` in a box far taller than its text, so
nothing shrinks and the pitch drawn is the rule alone. One box per font size, one size per
slide so the sacrificial first shape cannot leak state into more than its own slide.

The rule under test is EditEngine's font-independent line spacing,
`ImplCalculateFontIndependentLineSpacing` = `fround(fontHeight * 12 / 10)` on a font height
held in hundredths of a millimetre.
"""
import os
import sys
import zipfile

SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tmpl")
OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/pitch-probe.pptx"

EMU_PT = 12700

# Hundredths of a point, as `a:rPr/@sz` states them.
SIZES = [int(round(float(s) * 100)) for s in sys.argv[2].split(",")] if len(sys.argv) > 2 else [
    600, 800, 900, 1000, 1100, 1200, 1400, 1600, 1800, 2000, 2400, 2800, 3200, 4000]

FACE = os.environ.get("PROBE_FACE", "Liberation Sans")

SLIDE_NS = ('<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
            'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
            'xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">')

WORDS = [("Alpha", "Bravo"), ("Charlie", "Delta"), ("Echo", "Foxtrot")]


def para(words, sz):
    rpr = f'<a:rPr lang="en-GB" sz="{sz}"><a:latin typeface="{FACE}"/></a:rPr>'
    brk = f'<a:br>{rpr}</a:br>'
    return ('<a:p><a:pPr algn="l"/>'
            f'<a:r>{rpr}<a:t>{words[0]}</a:t></a:r>{brk}'
            f'<a:r>{rpr}<a:t>{words[1]}</a:t></a:r></a:p>')


def shape(sid, name, x, y, cx, cy, paras, autofit=False):
    return (
        f'<p:sp><p:nvSpPr><p:cNvPr id="{sid}" name="{name}"/><p:cNvSpPr txBox="1"/>'
        f'<p:nvPr/></p:nvSpPr><p:spPr><a:xfrm><a:off x="{x}" y="{y}"/>'
        f'<a:ext cx="{cx}" cy="{cy}"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
        '<a:noFill/></p:spPr><p:txBody>'
        '<a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0" anchor="t">'
        + ('<a:normAutofit/>' if autofit else '<a:noAutofit/>')
        + '</a:bodyPr><a:lstStyle/>' + "".join(paras) + '</p:txBody></p:sp>')


def build():
    slides = []
    for sz in SIZES:
        shapes = [
            # Sacrificial: the shared draw outliner formats the first text object on a page
            # before SetFixedCellHeight takes hold, so nothing under test may be first.
            shape(2, "warmup", 20 * EMU_PT, 5 * EMU_PT, 200 * EMU_PT, 30 * EMU_PT,
                  [para(("Warm", "Up"), 1200)]),
            shape(3, f"no{sz}", 20 * EMU_PT, 60 * EMU_PT, 300 * EMU_PT, 420 * EMU_PT,
                  [para(w, sz) for w in WORDS], autofit=False),
            shape(4, f"af{sz}", 380 * EMU_PT, 60 * EMU_PT, 300 * EMU_PT, 420 * EMU_PT,
                  [para(w, sz) for w in WORDS], autofit=True),
        ]
        slides.append(
            SLIDE_NS +
            '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/>'
            '<p:nvPr/></p:nvGrpSpPr><p:grpSpPr><a:xfrm><a:off x="0" y="0"/>'
            '<a:ext cx="0" cy="0"/><a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm>'
            '</p:grpSpPr>' + "".join(shapes) +
            '</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>')

    n = len(slides)
    if os.path.exists(OUT):
        os.remove(OUT)
    with zipfile.ZipFile(OUT, "w", zipfile.ZIP_DEFLATED) as z:
        for root, _, files in os.walk(SRC):
            for f in files:
                full = os.path.join(root, f)
                rel = os.path.relpath(full, SRC)
                if rel.startswith("ppt/slides/") or rel in (
                        "[Content_Types].xml", "ppt/presentation.xml",
                        "ppt/_rels/presentation.xml.rels"):
                    continue
                z.write(full, rel)

        ct = open(os.path.join(SRC, "[Content_Types].xml")).read()
        one = ('<Override PartName="/ppt/slides/slide1.xml" ContentType="application/'
               'vnd.openxmlformats-officedocument.presentationml.slide+xml"/>')
        extra = "".join(one.replace("slide1.xml", f"slide{i + 1}.xml") for i in range(n))
        # The template lists every slide it holds; drop them all, then add ours.
        for i in range(40, 0, -1):
            ct = ct.replace(one.replace("slide1.xml", f"slide{i}.xml"), "")
        ct = ct.replace("</Types>", extra + "</Types>")
        z.writestr("[Content_Types].xml", ct)

        pres = open(os.path.join(SRC, "ppt/presentation.xml")).read()
        import re
        pres = re.sub(r'<p:sldIdLst>.*?</p:sldIdLst>',
                      "<p:sldIdLst>" + "".join(
                          f'<p:sldId id="{256 + i}" r:id="rId{100 + i}"/>'
                          for i in range(n)) + "</p:sldIdLst>", pres, flags=re.S)
        z.writestr("ppt/presentation.xml", pres)

        rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                'relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>'
                '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                'relationships/theme" Target="theme/theme1.xml"/>' +
                "".join(f'<Relationship Id="rId{100 + i}" Type="http://schemas.openxmlformats.org/'
                        f'officeDocument/2006/relationships/slide" Target="slides/slide{i + 1}.xml"/>'
                        for i in range(n)) + '</Relationships>')
        z.writestr("ppt/_rels/presentation.xml.rels", rels)

        srel = open(os.path.join(SRC, "ppt/slides/_rels/slide1.xml.rels")).read()
        for i, s in enumerate(slides):
            z.writestr(f"ppt/slides/slide{i + 1}.xml",
                       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' + s)
            z.writestr(f"ppt/slides/_rels/slide{i + 1}.xml.rels", srel)
    print(OUT, n, "slides, sizes", ",".join(str(s / 100) for s in SIZES), "face", FACE)


build()
