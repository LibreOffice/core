#!/usr/bin/env python3
"""Where 26.2.4.2 puts a character bullet, measured rather than read off `outliner.cxx`.

`SlideTextLayout.EmitMarker` transcribes `Outliner::StripBullet` and `ImpCalcBulletArea`:
a symbol bullet's baseline is the bullet box's bottom less the bullet font's descent, which
reduces to `lineHeight - textHeight/2 + (markerAscent - markerDescent)/2` below the line's top.
Worked through by hand on `Lepore.ppt` that arithmetic gives the bullet 0.96 pt ABOVE the
text baseline -- which is what we draw -- and 26.2.4.2 draws it 0.99 pt BELOW.  So either an
input differs or the rule does, and no amount of reading the C++ separates the two.

One box per slide, `tIns="0"`, `anchor="t"`, `a:noAutofit`, a single bulleted paragraph of
three lines joined by `a:br` so that the first baseline gives the ascent and the pitch gives
the line height, both against a box top this file chose.  The arms vary one thing each:

  size        the run's own size, which scales every term
  buFont      the bullet's face, which is the only source of (markerAscent - markerDescent)
  buSzPct     the bullet's size alone, which moves that term and nothing else
  lnSpc       the line height alone, which moves `lineHeight` and the ascent
  buAutoNum   the control: a NUMBER is drawn at nFirstLineMaxAscent, i.e. exactly on the
              text baseline, so this arm must come out at 0.000 under every hypothesis

    make-bullet-probe.py <out.pptx>
"""
import os, sys, zipfile, importlib.util

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location(
    "fitprobe", os.path.join(HERE, "..", "slides-r52", "make-fit-probe.py"))
fp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fp)

EMU_PT = fp.EMU_PT
BOX_X, BOX_Y = 200000, 900000            # 15.75 pt, 70.87 pt
BOX_W, BOX_H = 7000000, 4000000

# (label, run size in hundredths of a point, bullet xml, lnSpc xml)
def buchar(face, pct=None):
    sz = f'<a:buSzPct val="{pct}"/>' if pct else ''
    return f'{sz}<a:buFont typeface="{face}"/><a:buChar char="&#8226;"/>'


CASES = []
for size in (1200, 2000, 2400, 4000):
    CASES.append((f"size {size/100} arial", size, buchar("Arial"), ''))
for face in ("Times New Roman", "Wingdings", "Courier New", "Verdana"):
    CASES.append((f"face {face}", 2000, buchar(face), ''))
for pct in (50000, 75000, 100000, 125000, 200000):
    CASES.append((f"buSzPct {pct/1000}", 2000, buchar("Arial", pct), ''))
for lns in (80000, 100000, 150000, 200000):
    CASES.append((f"lnSpc {lns/1000}", 2000, buchar("Arial"),
                  f'<a:lnSpc><a:spcPct val="{lns}"/></a:lnSpc>'))
CASES.append(("control autonum", 2000, '<a:buFont typeface="Arial"/>'
              '<a:buAutoNum type="arabicPeriod"/>', ''))
CASES.append(("control nobullet", 2000, '<a:buNone/>', ''))
# A run in one face with the bullet in another separates the two metrics outright.
CASES.append(("text serif bullet arial", 2000, buchar("Arial"), '', "Times New Roman"))
CASES.append(("text arial bullet serif", 2000, buchar("Times New Roman"), '', "Arial"))


def slide(size, bu, lnspc, face="Arial"):
    lines = '<a:br/>'.join(
        f'<a:r><a:rPr lang="en-GB" sz="{size}"><a:latin typeface="{face}"/></a:rPr>'
        f'<a:t>Hxyq{n}</a:t></a:r>' for n in range(3))
    body = (f'<a:p><a:pPr marL="457200" indent="-457200">{lnspc}{bu}</a:pPr>{lines}</a:p>')
    parts = [
        # The spacer is not decoration: the first text object a page lays out is formatted
        # before SetFixedCellHeight takes hold.  See SlideAutofit's remarks.
        fp.shape(2, 'Spacer', 200000, 100000, 2000000, 400000,
                 fp.paragraph('spacer', 1200), '<a:noAutofit/>'),
        fp.shape(3, 'Measured', BOX_X, BOX_Y, BOX_W, BOX_H, body, '<a:noAutofit/>'),
    ]
    return fp.HEAD + ''.join(parts) + fp.TAIL


if __name__ == '__main__':
    out = sys.argv[1]
    ct, ids, rels = [fp.CT_HEAD], [], []
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
            f'<p:sldSz cx="{fp.SLIDE_W}" cy="{fp.SLIDE_H}"/>'
            '<p:notesSz cx="6858000" cy="9144000"/></p:presentation>')
    pres_rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                 '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/'
                 '2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>'
                 f'{"".join(rels)}</Relationships>')

    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ''.join(ct))
        z.writestr('_rels/.rels', fp.ROOT_RELS)
        z.writestr('ppt/presentation.xml', pres)
        z.writestr('ppt/_rels/presentation.xml.rels', pres_rels)
        z.writestr('ppt/slideLayouts/slideLayout1.xml', fp.LAYOUT)
        z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels', fp.LAYOUT_RELS)
        z.writestr('ppt/slideMasters/slideMaster1.xml', fp.MASTER)
        z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels', fp.MASTER_RELS)
        z.writestr('ppt/theme/theme1.xml', fp.THEME)
        for i, case in enumerate(CASES):
            face = case[4] if len(case) > 4 else "Arial"
            z.writestr(f'ppt/slides/slide{i + 1}.xml',
                       slide(case[1], case[2], case[3], face))
            z.writestr(f'ppt/slides/_rels/slide{i + 1}.xml.rels', fp.SLIDE_RELS)
    print(f'{out}: {len(CASES)} slides')
    for i, c in enumerate(CASES):
        print(f'  {i+1:3d}  {c[0]}')
