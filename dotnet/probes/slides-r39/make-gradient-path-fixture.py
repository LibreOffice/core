#!/usr/bin/env python3
"""Author slide-gradient-path.pptx: four slide backgrounds that separate the
three candidate readings of a:gradFill/a:path/a:fillToRect from one another."""
import sys, zipfile

W, H = 9144000, 6858000

CASES = [
    # (name, fillToRect attributes)
    ('clamped',   'l="50000" t="-80000" r="50000" b="180000"'),  # focus (50, -80) -> (50, 0)
    ('corner',    'l="100000" t="100000"'),                      # focus (100, 100)
    ('halfPerCent', 'l="0" t="0" r="99000" b="99000"'),          # focus (0.5, 0.5) -> (0, 0)
    ('onePerCent',  'l="0" t="0" r="98000" b="98000"'),          # focus (1, 1)
]

NS = ('xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main"')

HDR = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'

EMPTY_TREE = ('<p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/>'
              '</p:nvGrpSpPr><p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/>'
              '<a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr></p:spTree>')


def slide(fill_to_rect):
    bg = ('<p:bg><p:bgPr><a:gradFill flip="none" rotWithShape="1"><a:gsLst>'
          '<a:gs pos="0"><a:srgbClr val="FF0000"/></a:gs>'
          '<a:gs pos="100000"><a:srgbClr val="0000FF"/></a:gs></a:gsLst>'
          f'<a:path path="circle"><a:fillToRect {fill_to_rect}/></a:path>'
          '</a:gradFill><a:effectLst/></p:bgPr></p:bg>')
    return (HDR + f'<p:sld {NS}><p:cSld>' + bg + EMPTY_TREE +
            '</p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>')


SCHEME = ''.join(
    f'<a:{n}><a:srgbClr val="{v}"/></a:{n}>' for n, v in
    [('dk1', '000000'), ('lt1', 'FFFFFF'), ('dk2', '44546A'), ('lt2', 'E7E6E6'),
     ('accent1', '4472C4'), ('accent2', 'ED7D31'), ('accent3', 'A5A5A5'),
     ('accent4', 'FFC000'), ('accent5', '5B9BD5'), ('accent6', '70AD47'),
     ('hlink', '0563C1'), ('folHlink', '954F72')])

FONTS = ('<a:latin typeface="Calibri"/><a:ea typeface=""/><a:cs typeface=""/>')

THEME = (HDR +
         '<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="t">'
         '<a:themeElements><a:clrScheme name="c">' + SCHEME + '</a:clrScheme>'
         '<a:fontScheme name="f"><a:majorFont>' + FONTS + '</a:majorFont>'
         '<a:minorFont>' + FONTS + '</a:minorFont></a:fontScheme>'
         '<a:fmtScheme name="s">'
         '<a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
         '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
         '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>'
         '<a:lnStyleLst><a:ln w="6350"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
         '</a:ln><a:ln w="12700"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>'
         '<a:ln w="19050"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>'
         '</a:lnStyleLst>'
         '<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle>'
         '<a:effectStyle><a:effectLst/></a:effectStyle>'
         '<a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>'
         '<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
         '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
         '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>'
         '</a:fmtScheme></a:themeElements></a:theme>')

MASTER = (HDR + f'<p:sldMaster {NS}><p:cSld>' + EMPTY_TREE + '</p:cSld>'
          '<p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1" '
          'accent2="accent2" accent3="accent3" accent4="accent4" accent5="accent5" '
          'accent6="accent6" hlink="hlink" folHlink="folHlink"/>'
          '<p:sldLayoutIdLst><p:sldLayoutId id="2147483649" r:id="rId1"/></p:sldLayoutIdLst>'
          '</p:sldMaster>')

LAYOUT = (HDR + f'<p:sldLayout {NS} type="blank" preserve="1"><p:cSld name="Blank">' +
          EMPTY_TREE + '</p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>')


def rels(items):
    body = ''.join(
        f'<Relationship Id="{i}" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
        f'relationships/{t}" Target="{g}"/>' for i, t, g in items)
    return (HDR + '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/'
            f'relationships">{body}</Relationships>')


def build(path):
    n = len(CASES)
    ids = ''.join(f'<p:sldId id="{256 + i}" r:id="rId{i + 2}"/>' for i in range(n))
    presentation = (HDR + f'<p:presentation {NS}>'
                    '<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/>'
                    '</p:sldMasterIdLst>'
                    f'<p:sldIdLst>{ids}</p:sldIdLst>'
                    f'<p:sldSz cx="{W}" cy="{H}"/><p:notesSz cx="{H}" cy="{W}"/>'
                    '</p:presentation>')

    ct = (HDR + '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
          '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
          'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
          '<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.'
          'openxmlformats-officedocument.presentationml.presentation.main+xml"/>'
          '<Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/'
          'vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>'
          '<Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/'
          'vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>'
          '<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.'
          'openxmlformats-officedocument.theme+xml"/>' +
          ''.join(f'<Override PartName="/ppt/slides/slide{i + 1}.xml" ContentType="application/'
                  'vnd.openxmlformats-officedocument.presentationml.slide+xml"/>'
                  for i in range(n)) + '</Types>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ct)
        z.writestr('_rels/.rels', rels([('rId1', 'officeDocument', 'ppt/presentation.xml')]))
        z.writestr('ppt/presentation.xml', presentation)
        z.writestr('ppt/_rels/presentation.xml.rels', rels(
            [('rId1', 'slideMaster', 'slideMasters/slideMaster1.xml')] +
            [(f'rId{i + 2}', 'slide', f'slides/slide{i + 1}.xml') for i in range(n)] +
            [(f'rId{n + 2}', 'theme', 'theme/theme1.xml')]))
        z.writestr('ppt/slideMasters/slideMaster1.xml', MASTER)
        z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels', rels(
            [('rId1', 'slideLayout', '../slideLayouts/slideLayout1.xml'),
             ('rId2', 'theme', '../theme/theme1.xml')]))
        z.writestr('ppt/slideLayouts/slideLayout1.xml', LAYOUT)
        z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels', rels(
            [('rId1', 'slideMaster', '../slideMasters/slideMaster1.xml')]))
        z.writestr('ppt/theme/theme1.xml', THEME)
        for i, (_, ftr) in enumerate(CASES):
            z.writestr(f'ppt/slides/slide{i + 1}.xml', slide(ftr))
            z.writestr(f'ppt/slides/_rels/slide{i + 1}.xml.rels', rels(
                [('rId1', 'slideLayout', '../slideLayouts/slideLayout1.xml')]))
    print(path, [c[0] for c in CASES])


build(sys.argv[1])
