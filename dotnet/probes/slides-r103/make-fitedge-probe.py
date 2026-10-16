#!/usr/bin/env python3
"""Build a deck whose every slide is the SAME autofitted body in a box of a stated
height, so the height at which 26.2.4.2 changes its mind can be bisected.

Why a threshold sweep rather than a scale comparison.  `ImpEditEngine::ScaleContentToFitWindow`
takes the FIRST row of `constScaleLevels` whose formatted block height fits the box, and the
comparison is `height > available` with `available = box + 1` (tools::Rectangle counts both
edges).  So if H is the box height in hundredths of a millimetre and the answer at H is row k
while the answer at H-1 is row k+1, then

    block(k) = H + 1                       exactly, in the renderer's own units.

That turns a quantity neither renderer prints into one both of them can be asked for through
a PDF, with nothing but the drawn size and the drawn baseline pitch read out of the content
stream.  Run it against 26.2.4.2 and against this tree and the two block heights are directly
comparable -- which is what O15 needs and what a per-page dominant-size census cannot give.

    make-fitedge-probe.py <out.pptx> --spec <spec.json>

where spec.json is [{"case": "<id>", "h": <box height, 1/100 mm>}, ...], one slide each.
"""
import argparse, json, zipfile, sys, os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from _parts import (CT_HEAD, ROOT_RELS, SLIDE_RELS, LAYOUT, LAYOUT_RELS,
                    MASTER, MASTER_RELS, THEME, SLIDE_W, SLIDE_H)

MM100 = 360                      # EMU per 1/100 mm
BOX_W = 22000                    # 1/100 mm -- inside the 25400 page, so wrapping is real
BOX_X = 1600
BOX_Y = 3000

HEAD = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"'
        ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
        ' xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">'
        '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/>'
        '<p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>')
TAIL = '</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>'

# RESPA_-_Section_8_Webinar.ppt page 18's outline, which is O15's own next witness: seven
# paragraphs of 24 pt with 6 pt above and below (0.212 cm in 26.2.4.2's own flat ODP of it),
# four of them wrapping.  `respa` is that body; the others peel one feature off it at a time.
RESPA = [
    "Agreement between Lighthouse and brokers to provide marketing services to Lighthouse.",
    "CFPB alleged that the payments were actually based on the number of referrals.",
    "Argued that they entered the MSA as a quid pro quo for referrals.",
    "How did they get to that conclusion?",
    "Analyzed referral business - brokers who were in an MSA with Lighthouse were far more "
    "likely to refer business to Lighthouse than brokers who were not.",
    "$200,000 fine to the CFPB",
    "https://www.consumerfinance.gov/policy-compliance/enforcement/actions/lighthouse-title/",
]
SHORT = [f"Line number {i} of the body" for i in range(1, 8)]

CASES = {
    # id            texts    size  spc(pt)  lnSpc(%)  bullet  autofit  typeface
    'plain':       (SHORT,   2400, 0,       0,        False,  True,  'Liberation Sans'),
    'spaced':      (SHORT,   2400, 6,       0,        False,  True,  'Liberation Sans'),
    'respa':       (RESPA,   2400, 6,       0,        False,  True,  'Liberation Sans'),
    'respa-nospc': (RESPA,   2400, 0,       0,        False,  True,  'Liberation Sans'),
    'prop90':      (SHORT,   2400, 6,       90,       False,  True,  'Liberation Sans'),
    'bullet':      (SHORT,   2400, 6,       0,        True,   True,  'Liberation Sans'),
    'mixed':       (SHORT,   1800, 4,       0,        False,  True,  'Liberation Sans'),
}

# The width sweep: one paragraph per case in a box that does NOT autofit, so the only
# threshold in the sweep is the box width at which the paragraph stops fitting on one line.
# That width, less one, is the advance the renderer gives the string -- in its own units,
# read out of the PDF without measuring a single glyph box.
WRAPS = {
    # RESPA page 18's own paragraphs, at the two sizes the fit chooses between there
    'w-respa2-19': ([RESPA[1]], 1900, 'DejaVu Sans'),
    'w-respa2-20': ([RESPA[1]], 2000, 'DejaVu Sans'),
    'w-respa1-19': ([RESPA[0]], 1900, 'DejaVu Sans'),
    'w-respa1-20': ([RESPA[0]], 2000, 'DejaVu Sans'),
    'w-respa5-20': ([RESPA[4]], 2000, 'DejaVu Sans'),
    'w-lib24':     ([RESPA[1]], 2400, 'Liberation Sans'),
    'w-lib20':     ([RESPA[1]], 2000, 'Liberation Sans'),
    'w-lib12':     ([RESPA[1]], 1200, 'Liberation Sans'),
    'w-car20':     ([RESPA[1]], 2000, 'Carlito'),
    'w-dv12':      ([RESPA[1]], 1200, 'DejaVu Sans'),
}
for _k, (_t, _s, _f) in WRAPS.items():
    CASES[_k] = (_t, _s, 0, 0, False, False, _f)


def para(text, size, spc, lnspc, bullet, typeface='Liberation Sans'):
    ln = f'<a:lnSpc><a:spcPct val="{lnspc * 1000}"/></a:lnSpc>' if lnspc else ''
    sp = (f'<a:spcBef><a:spcPts val="{spc * 100}"/></a:spcBef>'
          f'<a:spcAft><a:spcPts val="{spc * 100}"/></a:spcAft>') if spc else ''
    bu = ('<a:buFont typeface="StarSymbol"/><a:buChar char="•"/>' if bullet
          else '<a:buNone/>')
    marl = ' marL="457200" indent="-457200"' if bullet else ''
    text = text.replace('&', '&amp;').replace('<', '&lt;')
    return (f'<a:p><a:pPr{marl}>{ln}{sp}{bu}</a:pPr>'
            f'<a:r><a:rPr lang="en-GB" sz="{size}">'
            f'<a:latin typeface="{typeface}"/></a:rPr><a:t>{text}</a:t></a:r></a:p>')


def shape(idx, name, x, y, cx, cy, body, fit):
    return (f'<p:sp><p:nvSpPr><p:cNvPr id="{idx}" name="{name}"/>'
            f'<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
            f'<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
            f'<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0"'
            f' anchor="t">{fit}</a:bodyPr><a:lstStyle/>{body}</p:txBody></p:sp>')


def slide(case, height, width=BOX_W):
    texts, size, spc, lnspc, bullet, autofit, typeface = CASES[case]
    body = ''.join(para(t, size, spc, lnspc, bullet, typeface) for t in texts)
    return (HEAD
            # a spacer goes first because the first text object a page lays out is formatted
            # before SetFixedCellHeight takes hold -- see SlideAutofit's remarks.
            + shape(2, 'Spacer', 200000, 60000, 2000000, 300000,
                    para('spacer', 1200, 0, 0, False), '<a:noAutofit/>')
            + shape(3, 'FIT', BOX_X * MM100, BOX_Y * MM100, width * MM100, height * MM100,
                    body, '<a:normAutofit/>' if autofit else '<a:noAutofit/>')
            + TAIL)


def build(out, spec):
    ct = [CT_HEAD]; ids = []; rels = []
    for i in range(len(spec)):
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
                 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/'
                 '2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>'
                 f'{"".join(rels)}</Relationships>')
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ''.join(ct))
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('ppt/presentation.xml', pres)
        z.writestr('ppt/_rels/presentation.xml.rels', pres_rels)
        z.writestr('ppt/slideLayouts/slideLayout1.xml', LAYOUT)
        z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels', LAYOUT_RELS)
        z.writestr('ppt/slideMasters/slideMaster1.xml', MASTER)
        z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels', MASTER_RELS)
        z.writestr('ppt/theme/theme1.xml', THEME)
        for i, item in enumerate(spec):
            z.writestr(f'ppt/slides/slide{i+1}.xml',
                       slide(item['case'], item.get('h', 8000), item.get('w', BOX_W)))
            z.writestr(f'ppt/slides/_rels/slide{i+1}.xml.rels', SLIDE_RELS)


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('out'); ap.add_argument('--spec', required=True)
    a = ap.parse_args()
    with open(a.spec) as fh:
        spec = json.load(fh)
    build(a.out, spec)
    print(f'{a.out}: {len(spec)} slides')
