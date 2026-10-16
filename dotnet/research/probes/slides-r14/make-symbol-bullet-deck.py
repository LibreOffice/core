"""Build `tests/corpus/features/slide-symbol-bullet.pptx`.

Eight bulleted paragraphs, each stating a bullet in a legacy symbol face, chosen so the
deck covers every distinct behaviour of the recode table rather than eight instances of
one:

  * the two slots the corpus's largest symbol-bulleted decks actually use, one spelled
    in the Private Use Area (`&#xF0D8;`, what `2015-Civil-Rights-Website-training.ppt`
    bullets with) and one as a plain byte (`ü` = 0xFC, the commonest in the whole track
    at 266 uses),
  * the same slot through two different faces, which is the whole reason the table is
    keyed on the face at all,
  * a table hole, which LibreOffice substitutes rather than leaving as .notdef,
  * a symbol face with no table, which must still fall back to U+2022,
  * a non-symbol face, which must not be recoded at all.

Both spellings matter: a bullet stated as `&#xF0D8;` and one stated as `Ø` are the same
slot, and this round shipped a bug that handled the second correctly and the first not,
which no fixture carrying only one of them could have caught.

Run from anywhere; writes into this checkout's own corpus.
"""
import os
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__)))))
OUT = os.path.join(ROOT, 'tests/corpus/features/slide-symbol-bullet.pptx')

A = "http://schemas.openxmlformats.org/drawingml/2006/main"
P = "http://schemas.openxmlformats.org/presentationml/2006/main"
R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
NS = f'xmlns:a="{A}" xmlns:r="{R}" xmlns:p="{P}"'
X = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'

SCHEME = ('<a:clrScheme name="Office">'
          '<a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>'
          '<a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1>'
          '<a:dk2><a:srgbClr val="1F497D"/></a:dk2><a:lt2><a:srgbClr val="EEECE1"/></a:lt2>'
          '<a:accent1><a:srgbClr val="4F81BD"/></a:accent1>'
          '<a:accent2><a:srgbClr val="C0504D"/></a:accent2>'
          '<a:accent3><a:srgbClr val="9BBB59"/></a:accent3>'
          '<a:accent4><a:srgbClr val="8064A2"/></a:accent4>'
          '<a:accent5><a:srgbClr val="4BACC6"/></a:accent5>'
          '<a:accent6><a:srgbClr val="F79646"/></a:accent6>'
          '<a:hlink><a:srgbClr val="0000FF"/></a:hlink>'
          '<a:folHlink><a:srgbClr val="800080"/></a:folHlink></a:clrScheme>')

FONTS = ('<a:fontScheme name="Office">'
         '<a:majorFont><a:latin typeface="Cambria"/><a:ea typeface=""/><a:cs typeface=""/></a:majorFont>'
         '<a:minorFont><a:latin typeface="Calibri"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont>'
         '</a:fontScheme>')

FILL = '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
LINE = ('<a:ln w="9525" cap="flat" cmpd="sng" algn="ctr"><a:solidFill>'
        '<a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/></a:ln>')
FMT = ('<a:fmtScheme name="Office">'
       f'<a:fillStyleLst>{FILL}{FILL}{FILL}</a:fillStyleLst>'
       f'<a:lnStyleLst>{LINE}{LINE}{LINE}</a:lnStyleLst>'
       '<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle>'
       '<a:effectStyle><a:effectLst/></a:effectStyle>'
       '<a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>'
       f'<a:bgFillStyleLst>{FILL}{FILL}{FILL}</a:bgFillStyleLst></a:fmtScheme>')

theme = (X + f'<a:theme {NS} name="Office"><a:themeElements>'
         + SCHEME + FONTS + FMT
         + '</a:themeElements><a:objectDefaults/><a:extraClrSchemeLst/></a:theme>')

empty_tree = ('<p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>'
              '<p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/>'
              '<a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr></p:spTree>')

master = (X + f'<p:sldMaster {NS}><p:cSld>{empty_tree}</p:cSld>'
          '<p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1" accent2="accent2"'
          ' accent3="accent3" accent4="accent4" accent5="accent5" accent6="accent6"'
          ' hlink="hlink" folHlink="folHlink"/>'
          '<p:sldLayoutIdLst><p:sldLayoutId id="2147483649" r:id="rId1"/></p:sldLayoutIdLst>'
          '<p:txStyles><p:titleStyle/><p:bodyStyle/><p:otherStyle/></p:txStyles></p:sldMaster>')

layout = (X + f'<p:sldLayout {NS} type="obj" preserve="1"><p:cSld name="Content">{empty_tree}'
          '</p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>')

# (label, buFont typeface, charset attribute, the char as XML, what it exercises)
CASES = [
    ('Wingdings F0D8 arrowhead',  'Wingdings',      '2', '&#xF0D8;'),
    ('Wingdings 00D8 same slot',  'Wingdings',      '2', '&#xD8;'),
    ('Wingdings 00FC commonest',  'Wingdings',      '2', '&#xFC;'),
    ('Wingdings F06E circle',     'Wingdings',      '2', '&#xF06E;'),
    ('Symbol F0B7 bullet',        'Symbol',         '2', '&#xF0B7;'),
    ('Monotype Sorts F0D8',       'Monotype Sorts', '2', '&#xF0D8;'),
    ('Wingdings F07F table hole', 'Wingdings',      '2', '&#xF07F;'),
    ('Unknown dingbat no table',  'Private Dingbat', '2', '&#xF0D8;'),
    ('Calibri plain bullet',      'Calibri',        '0', '&#x2022;'),
]


def shape(index, label, face, charset, character):
    y = 300000 + index * 620000
    return (f'<p:sp><p:nvSpPr><p:cNvPr id="{index + 2}" name="Box{index + 1}"/>'
            '<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
            f'<p:spPr><a:xfrm><a:off x="500000" y="{y}"/><a:ext cx="8000000" cy="520000"/></a:xfrm>'
            '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
            '<p:txBody><a:bodyPr wrap="none"><a:noAutofit/></a:bodyPr><a:lstStyle/>'
            '<a:p><a:pPr marL="457200" indent="-457200">'
            f'<a:buFont typeface="{face}" charset="{charset}"/>'
            f'<a:buChar char="{character}"/></a:pPr>'
            f'<a:r><a:rPr lang="en-GB" sz="2000"/><a:t>{label}</a:t></a:r></a:p>'
            '</p:txBody></p:sp>')


shapes = ''.join(shape(i, *case) for i, case in enumerate(CASES))

slide = (X + f'<p:sld {NS}><p:cSld><p:spTree>'
         '<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>'
         '<p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/>'
         '<a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr>'
         + shapes +
         '</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>')

presentation = (X + f'<p:presentation {NS}>'
                '<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/></p:sldMasterIdLst>'
                '<p:sldIdLst><p:sldId id="256" r:id="rId2"/></p:sldIdLst>'
                '<p:sldSz cx="9144000" cy="6858000"/><p:notesSz cx="6858000" cy="9144000"/>'
                '</p:presentation>')


def rels(*entries):
    body = ''.join(
        f'<Relationship Id="{i}" Type="{t}" Target="{g}"/>' for i, t, g in entries)
    return (X + '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            + body + '</Relationships>')


T = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/"
content_types = (X + '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
                 '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
                 '<Default Extension="xml" ContentType="application/xml"/>'
                 '<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>'
                 '<Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>'
                 '<Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>'
                 '<Override PartName="/ppt/slides/slide1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>'
                 '<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>'
                 '</Types>')

parts = {
    '[Content_Types].xml': content_types,
    '_rels/.rels': rels(("rId1", T + "officeDocument", "ppt/presentation.xml")),
    'ppt/presentation.xml': presentation,
    'ppt/_rels/presentation.xml.rels': rels(
        ("rId1", T + "slideMaster", "slideMasters/slideMaster1.xml"),
        ("rId2", T + "slide", "slides/slide1.xml"),
        ("rId3", T + "theme", "theme/theme1.xml")),
    'ppt/slideMasters/slideMaster1.xml': master,
    'ppt/slideMasters/_rels/slideMaster1.xml.rels': rels(
        ("rId1", T + "slideLayout", "../slideLayouts/slideLayout1.xml"),
        ("rId2", T + "theme", "../theme/theme1.xml")),
    'ppt/slideLayouts/slideLayout1.xml': layout,
    'ppt/slideLayouts/_rels/slideLayout1.xml.rels': rels(
        ("rId1", T + "slideMaster", "../slideMasters/slideMaster1.xml")),
    'ppt/slides/slide1.xml': slide,
    'ppt/slides/_rels/slide1.xml.rels': rels(
        ("rId1", T + "slideLayout", "../slideLayouts/slideLayout1.xml")),
    'ppt/theme/theme1.xml': theme,
}

os.makedirs(os.path.dirname(OUT), exist_ok=True)
if os.path.exists(OUT):
    os.remove(OUT)
with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED) as z:
    for name, text in parts.items():
        z.writestr(name, text)
print(OUT)
