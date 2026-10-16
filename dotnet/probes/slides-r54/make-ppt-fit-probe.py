#!/usr/bin/env python3
"""Author a .pptx whose fitted box is a BODY placeholder, so `soffice --convert-to ppt`
writes a TextHeaderAtom of instance Body and the binary import turns autofit on.

Round 53's brief asks for a known-answer `.ppt` autofit deck.  `probes/slides-r52/
make-fit-probe.py` authors plain text boxes, and those round-trip to `kind=4`
(TextInShape), which `svdfppt.cxx:1030-1039` gives NO autofit: measured, the exported
`.ppt` draws every slide at the stated 40 pt over 21 overflowing lines.  Autofit is not
spelled anywhere in the binary format -- it is inferred from the text KIND -- so the probe
has to be authored as a placeholder to survive the round trip.

One slide per box height.  A 12 pt spacer text box goes first on every slide for the same
reason `make-fit-probe.py` uses one.

    make-ppt-fit-probe.py <out.pptx> [--size 4000] [--from-height 60] [--to-height 200]
                          [--step 10] [--width 360] [--paras 3] [--lnspc N]
"""
import argparse, zipfile

EMU_PT = 12700
SLIDE_W, SLIDE_H = 9144000, 6858000     # 720 x 540 pt

WORDS = ("Proficient in more than one language and able to convey meaning "
         "accurately between two parties without adding or omitting anything")

NS = ('xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main"')

ROOT_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="ppt/presentation.xml"/>
</Relationships>'''

SLIDE_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
</Relationships>'''

LAYOUT_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="../slideMasters/slideMaster1.xml"/>
</Relationships>'''

MASTER_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="../theme/theme1.xml"/>
</Relationships>'''


def ph_shape(idx, name, ph, x, y, cx, cy, body, fit):
    return (f'<p:sp><p:nvSpPr><p:cNvPr id="{idx}" name="{name}"/>'
            f'<p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr><p:nvPr>{ph}</p:nvPr></p:nvSpPr>'
            f'<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
            f'<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0"'
            f' anchor="t">{fit}</a:bodyPr><a:lstStyle/>{body}</p:txBody></p:sp>')


def box_shape(idx, name, x, y, cx, cy, body, fit):
    return (f'<p:sp><p:nvSpPr><p:cNvPr id="{idx}" name="{name}"/>'
            f'<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
            f'<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
            f'<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0"'
            f' anchor="t">{fit}</a:bodyPr><a:lstStyle/>{body}</p:txBody></p:sp>')


def paragraph(text, size, lnspc):
    ln = f'<a:lnSpc><a:spcPct val="{lnspc}"/></a:lnSpc>' if lnspc else ''
    return (f'<a:p><a:pPr marL="0" indent="0">{ln}<a:buNone/></a:pPr>'
            f'<a:r><a:rPr lang="en-GB" sz="{size}"><a:latin typeface="Liberation Sans"/></a:rPr>'
            f'<a:t>{text}</a:t></a:r></a:p>')


THEME = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="T"><a:themeElements>
<a:clrScheme name="T"><a:dk1><a:srgbClr val="000000"/></a:dk1><a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>
<a:dk2><a:srgbClr val="000000"/></a:dk2><a:lt2><a:srgbClr val="FFFFFF"/></a:lt2>''' + "".join(
    f'<a:{n}><a:srgbClr val="4472C4"/></a:{n}>' for n in
    ["accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"]) + '''</a:clrScheme>
<a:fontScheme name="T"><a:majorFont><a:latin typeface="Liberation Sans"/><a:ea typeface=""/><a:cs typeface=""/></a:majorFont>
<a:minorFont><a:latin typeface="Liberation Sans"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont></a:fontScheme>
<a:fmtScheme name="T"><a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>
<a:lnStyleLst><a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln><a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln><a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>
<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>
<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst></a:fmtScheme>
</a:themeElements></a:theme>'''

BODY_LVL = ''.join(
    f'<a:lvl{i}pPr marL="0" indent="0"><a:buNone/>'
    f'<a:defRPr sz="1800"><a:latin typeface="Liberation Sans"/></a:defRPr></a:lvl{i}pPr>'
    for i in range(1, 10))

MASTER = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
          f'<p:sldMaster {NS}>'
          '<p:cSld><p:bg><p:bgPr><a:solidFill><a:srgbClr val="FFFFFF"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>'
          '<p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>'
          + ph_shape(2, 'Body Placeholder 1', '<p:ph type="body" idx="1"/>',
                     200000, 700000, 4572000, 2000000,
                     '<a:p><a:endParaRPr lang="en-GB"/></a:p>', '<a:normAutofit/>')
          + '</p:spTree></p:cSld>'
          '<p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1" accent2="accent2"'
          ' accent3="accent3" accent4="accent4" accent5="accent5" accent6="accent6" hlink="hlink" folHlink="folHlink"/>'
          '<p:sldLayoutIdLst><p:sldLayoutId id="2147483649" r:id="rId1"/></p:sldLayoutIdLst>'
          f'<p:txStyles><p:titleStyle/><p:bodyStyle>{BODY_LVL}</p:bodyStyle><p:otherStyle/></p:txStyles>'
          '</p:sldMaster>')

LAYOUT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
          f'<p:sldLayout {NS} type="obj" preserve="1">'
          '<p:cSld name="Title and Content"><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/>'
          '<p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>'
          + ph_shape(2, 'Content Placeholder 1', '<p:ph type="body" idx="1"/>',
                     200000, 700000, 4572000, 2000000,
                     '<a:p><a:endParaRPr lang="en-GB"/></a:p>', '<a:normAutofit/>')
          + '</p:spTree></p:cSld>'
          '<p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>')

HEAD = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<p:sld {NS}><p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/>'
        '<p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>')
TAIL = '</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>'


def slide(height_pt, width_pt, size, paras, lnspc):
    body = ''.join(paragraph(WORDS, size, lnspc) for _ in range(paras))
    return HEAD + box_shape(2, 'Spacer', 200000, 100000, 2000000, 400000,
                            paragraph('spacer', 1200, 0), '<a:noAutofit/>') \
                + ph_shape(3, 'Content Placeholder 1', '<p:ph type="body" idx="1"/>',
                           200000, 700000, int(width_pt * EMU_PT), int(height_pt * EMU_PT),
                           body, '<a:normAutofit/>') + TAIL


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('out')
    ap.add_argument('--size', type=int, default=4000, help='hundredths of a point')
    ap.add_argument('--from-height', type=float, default=60.0)
    ap.add_argument('--to-height', type=float, default=200.0)
    ap.add_argument('--step', type=float, default=10.0)
    ap.add_argument('--width', type=float, default=360.0)
    ap.add_argument('--paras', type=int, default=3)
    ap.add_argument('--lnspc', type=int, default=0,
                    help='a:lnSpc/a:spcPct on every paragraph, thousandths of a per cent')
    a = ap.parse_args()

    heights, h = [], a.from_height
    while h <= a.to_height + 1e-9:
        heights.append(round(h, 2))
        h += a.step

    ct = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
          '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
          '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
          '<Default Extension="xml" ContentType="application/xml"/>'
          '<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>'
          '<Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>'
          '<Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>'
          '<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>']
    ids, rels = [], []
    for i in range(len(heights)):
        n = i + 1
        ct.append(f'<Override PartName="/ppt/slides/slide{n}.xml" ContentType='
                  f'"application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>')
        ids.append(f'<p:sldId id="{255 + n}" r:id="rId{100 + n}"/>')
        rels.append(f'<Relationship Id="rId{100 + n}" Type="http://schemas.openxmlformats.org'
                    f'/officeDocument/2006/relationships/slide" Target="slides/slide{n}.xml"/>')
    ct.append('</Types>')

    pres = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<p:presentation {NS}>'
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
        for i, hh in enumerate(heights):
            z.writestr(f'ppt/slides/slide{i + 1}.xml',
                       slide(hh, a.width, a.size, a.paras, a.lnspc))
            z.writestr(f'ppt/slides/_rels/slide{i + 1}.xml.rels', SLIDE_RELS)
    print(f'{a.out}: {len(heights)} slides, heights {heights[0]}..{heights[-1]} pt, '
          f'size {a.size / 100} pt, {a.paras} paragraphs, lnSpc {a.lnspc or "none"}')
