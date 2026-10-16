#!/usr/bin/env python3
"""One deck that discriminates every line-height claim `SlideTextLayout.cs` still dates to 24.2.7.

Four of that file's six `24.2.7` sites are the EditEngine line-height arithmetic:

  :792  the `Off` branch -- a paragraph stating no proportion gets an ascent of one em and
        no four-fifths (`impedit3.cxx:1553-1602`)
  :1007 the `Prop` branch -- `fround(height x proportion)` below 100%, a truncating
        `sal_Int32` above it (`impedit3.cxx:1553-1580`)
  :1095 the same rounding, stated again on `Proportioned` (`impedit3.cxx:1568,1575`)
  :1133 `ProportionedAscent` -- below 100% the ascent is CAPPED at
        `fround(txtHeight x factor x 0.8)` and never raised; above it moves by the whole of
        the height's change (`impedit3.cxx:1564-1578`)

All four are observable from one page without reading any source: the baseline PITCH inside a
paragraph gives the height, and the FIRST baseline's offset below the box's top edge gives the
ascent.  One box per slide, `tIns="0"`, `a:noAutofit` so no fit scale is in play, one paragraph
of four lines separated by `a:br` so the line count cannot move with the proportion.

    make-linespace-probe.py <out.pptx>
"""
import argparse, os, sys, zipfile
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "slides-r52"))
import importlib.util
spec = importlib.util.spec_from_file_location(
    "fitprobe",
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "slides-r52", "make-fit-probe.py"))
fp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fp)

EMU_PT = fp.EMU_PT
BOX_X, BOX_Y = 200000, 700000          # the box's top-left, in EMU
BOX_W, BOX_H = 6000000, 3600000        # tall enough that nothing overflows

# (label, size in hundredths of a point, lnSpc xml)
def pct(v):
    return f'<a:lnSpc><a:spcPct val="{v}"/></a:lnSpc>'

def pts(v):
    return f'<a:lnSpc><a:spcPts val="{v}"/></a:lnSpc>'

CASES = []
for size in (4000, 2400, 1200, 1100):
    CASES.append((f"{size/100}pt none", size, ''))
    for p in (40000, 50000, 60000, 80000, 90000, 93000, 100000, 110000, 150000, 200000):
        CASES.append((f"{size/100}pt {p/1000}%", size, pct(p)))
    for e in (1000, 2400, 5000):
        CASES.append((f"{size/100}pt exactly {e/100}pt", size, pts(e)))


def slide(size, lnspc):
    lines = '<a:br/>'.join(
        f'<a:r><a:rPr lang="en-GB" sz="{size}"><a:latin typeface="Liberation Sans"/></a:rPr>'
        f'<a:t>Hxy{n}</a:t></a:r>' for n in range(4))
    body = f'<a:p><a:pPr>{lnspc}</a:pPr>{lines}</a:p>'
    parts = [
        # The spacer is not decoration: the first text object a page lays out is formatted
        # before SetFixedCellHeight takes hold.  See SlideAutofit's remarks.
        fp.shape(2, 'Spacer', 200000, 100000, 2000000, 400000,
                 fp.paragraph('spacer', 1200), '<a:noAutofit/>'),
        fp.shape(3, 'Measured', BOX_X, BOX_Y, BOX_W, BOX_H, body, '<a:noAutofit/>'),
    ]
    return fp.HEAD + ''.join(parts) + fp.TAIL


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('out')
    a = ap.parse_args()

    ct = [fp.CT_HEAD]
    ids, rels = [], []
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
            f'<p:sldSz cx="{fp.SLIDE_W}" cy="{fp.SLIDE_H}"/><p:notesSz cx="6858000" cy="9144000"/>'
            '</p:presentation>')
    pres_rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                 '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>'
                 f'{"".join(rels)}</Relationships>')

    with zipfile.ZipFile(a.out, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ''.join(ct))
        z.writestr('_rels/.rels', fp.ROOT_RELS)
        z.writestr('ppt/presentation.xml', pres)
        z.writestr('ppt/_rels/presentation.xml.rels', pres_rels)
        z.writestr('ppt/slideLayouts/slideLayout1.xml', fp.LAYOUT)
        z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels', fp.LAYOUT_RELS)
        z.writestr('ppt/slideMasters/slideMaster1.xml', fp.MASTER)
        z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels', fp.MASTER_RELS)
        z.writestr('ppt/theme/theme1.xml', fp.THEME)
        for i, (label, size, lnspc) in enumerate(CASES):
            z.writestr(f'ppt/slides/slide{i + 1}.xml', slide(size, lnspc))
            z.writestr(f'ppt/slides/_rels/slide{i + 1}.xml.rels', fp.SLIDE_RELS)
    for i, (label, _, _) in enumerate(CASES):
        print(f'slide {i + 1}: {label}')
    print(f'{a.out}: {len(CASES)} slides')
