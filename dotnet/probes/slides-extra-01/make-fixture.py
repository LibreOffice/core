#!/usr/bin/env python3
"""Author `text-warp-deck.pptx`, the fixture for a:prstTxWarp.

One slide, three text boxes at round positions in Liberation Sans 18 pt so nothing is
substituted and every advance is one the reference measured:

  PlainBox   no a:bodyPr/a:prstTxWarp at all
  NoShapeBox a:prstTxWarp prst="textNoShape" -- the value that means *no* warp
  WarpedBox  a:prstTxWarp prst="textArchUp"  -- Fontwork

All three hold the same three words, so the reference's own word count says exactly how
many of the three it puts in its text layer. It draws two: LibreOffice converts a warped
body to polygon outlines (EnhancedCustomShapeFontWork) and those carry no glyph.

Written by hand rather than generated with make-corpus.sh because prstTxWarp has no ODF
spelling that survives a round trip through soffice --convert-to pptx.
"""

import sys, zipfile

NS = 'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"'
PNS = ('xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main" '
       'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')


def box(shape_id, name, x, y, warp):
    warp_xml = f'<a:prstTxWarp prst="{warp}"><a:avLst/></a:prstTxWarp>' if warp else ''
    return f'''<p:sp><p:nvSpPr><p:cNvPr id="{shape_id}" name="{name}"/>
<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>
<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="3200400" cy="800100"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>
<p:txBody><a:bodyPr wrap="square" rtlCol="0" lIns="0" tIns="0" rIns="0" bIns="0">{warp_xml}</a:bodyPr><a:lstStyle/>
<a:p><a:r><a:rPr lang="en-US" sz="1800" dirty="0"><a:latin typeface="Liberation Sans"/></a:rPr>
<a:t>Fontwork keeps three</a:t></a:r></a:p></p:txBody></p:sp>'''


SLIDE = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sld {PNS} {NS}><p:cSld><p:spTree>
<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
<p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/>
<a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr>
{box(2, "PlainBox", 914400, 914400, None)}
{box(3, "NoShapeBox", 914400, 2286000, "textNoShape")}
{box(4, "WarpedBox", 914400, 3657600, "textArchUp")}
</p:spTree></p:cSld><p:clrMapOvr><a:overrideClrMapping bg1="lt1" tx1="dk1" bg2="lt2"
 tx2="dk2" accent1="accent1" accent2="accent2" accent3="accent3" accent4="accent4"
 accent5="accent5" accent6="accent6" hlink="hlink" folHlink="folHlink"/></p:clrMapOvr></p:sld>'''

LAYOUT = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sldLayout {PNS} {NS} type="blank" preserve="1"><p:cSld name="Blank"><p:spTree>
<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
<p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/>
<a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr>
</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>'''

MASTER = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sldMaster {PNS} {NS}><p:cSld><p:bg><p:bgPr><a:solidFill><a:schemeClr val="bg1"/>
</a:solidFill><a:effectLst/></p:bgPr></p:bg><p:spTree>
<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
<p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/>
<a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr>
</p:spTree></p:cSld><p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1"
 accent2="accent2" accent3="accent3" accent4="accent4" accent5="accent5"
 accent6="accent6" hlink="hlink" folHlink="folHlink"/>
<p:sldLayoutIdLst><p:sldLayoutId id="2147483649" r:id="rId1"/></p:sldLayoutIdLst>
<p:txStyles><p:titleStyle/><p:bodyStyle/><p:otherStyle/></p:txStyles></p:sldMaster>'''

PRESENTATION = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:presentation {PNS} {NS}>
<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/></p:sldMasterIdLst>
<p:sldIdLst><p:sldId id="256" r:id="rId2"/></p:sldIdLst>
<p:sldSz cx="9144000" cy="6858000"/><p:notesSz cx="6858000" cy="9144000"/></p:presentation>'''


def scheme():
    fonts = ''.join(
        f'<a:{k}><a:latin typeface="Liberation Sans"/><a:ea typeface=""/><a:cs typeface=""/></a:{k}>'
        for k in ('majorFont', 'minorFont'))
    colours = ''.join(
        f'<a:{k}><a:srgbClr val="{v}"/></a:{k}>'
        for k, v in (('dk1', '000000'), ('lt1', 'FFFFFF'), ('dk2', '44546A'),
                     ('lt2', 'E7E6E6'), ('accent1', '4472C4'), ('accent2', 'ED7D31'),
                     ('accent3', 'A5A5A5'), ('accent4', 'FFC000'), ('accent5', '5B9BD5'),
                     ('accent6', '70AD47'), ('hlink', '0563C1'), ('folHlink', '954F72')))
    line = ('<a:ln w="9525" cap="flat" cmpd="sng" algn="ctr"><a:solidFill>'
            '<a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/></a:ln>')
    fill = '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
    return f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme {NS} name="X-Ray.Office"><a:themeElements>
<a:clrScheme name="X-Ray.Office">{colours}</a:clrScheme>
<a:fontScheme name="X-Ray.Office">{fonts}</a:fontScheme>
<a:fmtScheme name="X-Ray.Office">
<a:fillStyleLst>{fill}{fill}{fill}</a:fillStyleLst>
<a:lnStyleLst>{line}{line}{line}</a:lnStyleLst>
<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle>
<a:effectStyle><a:effectLst/></a:effectStyle>
<a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>
<a:bgFillStyleLst>{fill}{fill}{fill}</a:bgFillStyleLst>
</a:fmtScheme></a:themeElements></a:theme>'''


REL = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
OFF = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'


def rels(*items):
    body = ''.join(
        f'<Relationship Id="{i}" Type="{t}" Target="{g}"/>' for i, t, g in items)
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">{body}</Relationships>')


CONTENT_TYPES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>
<Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>
<Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>
<Override PartName="/ppt/slides/slide1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>
<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>
</Types>'''

PARTS = {
    '[Content_Types].xml': CONTENT_TYPES,
    '_rels/.rels': rels(('rId1', f'{OFF}/officeDocument', 'ppt/presentation.xml')),
    'ppt/presentation.xml': PRESENTATION,
    'ppt/_rels/presentation.xml.rels': rels(
        ('rId1', f'{OFF}/slideMaster', 'slideMasters/slideMaster1.xml'),
        ('rId2', f'{OFF}/slide', 'slides/slide1.xml'),
        ('rId3', f'{OFF}/theme', 'theme/theme1.xml')),
    'ppt/slideMasters/slideMaster1.xml': MASTER,
    'ppt/slideMasters/_rels/slideMaster1.xml.rels': rels(
        ('rId1', f'{OFF}/slideLayout', '../slideLayouts/slideLayout1.xml'),
        ('rId2', f'{OFF}/theme', '../theme/theme1.xml')),
    'ppt/slideLayouts/slideLayout1.xml': LAYOUT,
    'ppt/slideLayouts/_rels/slideLayout1.xml.rels': rels(
        ('rId1', f'{OFF}/slideMaster', '../slideMasters/slideMaster1.xml')),
    'ppt/slides/slide1.xml': SLIDE,
    'ppt/slides/_rels/slide1.xml.rels': rels(
        ('rId1', f'{OFF}/slideLayout', '../slideLayouts/slideLayout1.xml')),
    'ppt/theme/theme1.xml': scheme(),
}


def main(out):
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in PARTS.items():
            z.writestr(name, text)
    print(out)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else 'text-warp-deck.pptx')
