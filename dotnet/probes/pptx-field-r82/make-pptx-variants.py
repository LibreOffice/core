#!/usr/bin/env python3
"""Author one-attribute .pptx variants of a text box holding a long URL.

Each deck differs from `v1` in exactly one thing, so the reference's answer can be
attributed.  The box is 14 x 5 cm at (1 cm, 1 cm) with zero insets and no autofit,
the text 16 pt Liberation Sans -- the same geometry `probes/odp-visual-r80/
field-variants.py` used for the ODF twin, so the two sets of baselines compare.
"""
import os, sys, zipfile
from pathlib import Path

OUT = Path(sys.argv[1] if len(sys.argv) > 1 else '/tmp/pptxfield/variants')
OUT.mkdir(parents=True, exist_ok=True)

CM = 360000
URL = "https://www.example.org/hr-connect/organisational-development/career-and-development-planning-framework/leadership/"
SHORT = "https://example.org/"

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>
<Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>
<Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>
<Override PartName="/ppt/slides/slide1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>
<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="ppt/presentation.xml"/>
</Relationships>"""

PRES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:presentation xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/></p:sldMasterIdLst>
<p:sldIdLst><p:sldId id="256" r:id="rId2"/></p:sldIdLst>
<p:sldSz cx="9144000" cy="6858000"/><p:notesSz cx="6858000" cy="9144000"/>
</p:presentation>"""

PRES_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" Target="slides/slide1.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/>
</Relationships>"""

def theme():
    fonts = ''.join(
        f'<a:{k}><a:latin typeface="Liberation Sans"/><a:ea typeface=""/><a:cs typeface=""/></a:{k}>'
        for k in ('majorFont', 'minorFont'))
    scheme = ''.join(
        f'<a:{n}><a:srgbClr val="{v}"/></a:{n}>' for n, v in
        [('dk1','000000'),('lt1','FFFFFF'),('dk2','000000'),('lt2','FFFFFF'),
         ('accent1','4472C4'),('accent2','ED7D31'),('accent3','A5A5A5'),('accent4','FFC000'),
         ('accent5','5B9BD5'),('accent6','70AD47'),('hlink','0563C1'),('folHlink','954F72')])
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="p">
<a:themeElements><a:clrScheme name="p">{scheme}</a:clrScheme>
<a:fontScheme name="p">{fonts}</a:fontScheme>
<a:fmtScheme name="p">
<a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>
<a:lnStyleLst><a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln><a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln><a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>
<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>
<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>
</a:fmtScheme></a:themeElements></a:theme>"""

MASTER = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sldMaster xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
<p:grpSpPr/></p:spTree></p:cSld>
<p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1" accent2="accent2" accent3="accent3" accent4="accent4" accent5="accent5" accent6="accent6" hlink="hlink" folHlink="folHlink"/>
<p:sldLayoutIdLst><p:sldLayoutId id="2147483649" r:id="rId1"/></p:sldLayoutIdLst>
</p:sldMaster>"""

MASTER_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="../theme/theme1.xml"/>
</Relationships>"""

LAYOUT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sldLayout xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main" type="blank" preserve="1">
<p:cSld name="Blank"><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
<p:grpSpPr/></p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>"""

LAYOUT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="../slideMasters/slideMaster1.xml"/>
</Relationships>"""

def slide(body, anchor='t'):
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
<p:cSld><p:spTree>
<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>
<p:sp><p:nvSpPr><p:cNvPr id="2" name="box"/><p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>
<p:spPr><a:xfrm><a:off x="{CM}" y="{CM}"/><a:ext cx="{14*CM}" cy="{5*CM}"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>
<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0" anchor="{anchor}"><a:noAutofit/></a:bodyPr>
<a:lstStyle/>
{body}
</p:txBody></p:sp>
</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>"""

def rpr(link=None, extra=''):
    if link is None:
        return f'<a:rPr lang="en-US" sz="1600"{extra}><a:latin typeface="Liberation Sans"/></a:rPr>'
    return (f'<a:rPr lang="en-US" sz="1600"{extra}><a:latin typeface="Liberation Sans"/>'
            f'<a:hlinkClick xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" r:id="{link}"/></a:rPr>')

def run(text, link=None, extra=''):
    return f'<a:r>{rpr(link, extra)}<a:t>{text}</a:t></a:r>'

def para(runs):
    return f'<a:p><a:pPr><a:defRPr sz="1600"/></a:pPr>{runs}</a:p>'

SLIDE_RELS_LINK = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
<Relationship Id="rId9" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink" Target="https://www.example.org/" TargetMode="External"/>
</Relationships>"""

SLIDE_RELS_PLAIN = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
</Relationships>"""

def deck(path, body, anchor='t', rels=SLIDE_RELS_LINK):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('ppt/presentation.xml', PRES)
        z.writestr('ppt/_rels/presentation.xml.rels', PRES_RELS)
        z.writestr('ppt/theme/theme1.xml', theme())
        z.writestr('ppt/slideMasters/slideMaster1.xml', MASTER)
        z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels', MASTER_RELS)
        z.writestr('ppt/slideLayouts/slideLayout1.xml', LAYOUT)
        z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels', LAYOUT_RELS)
        z.writestr('ppt/slides/slide1.xml', slide(body, anchor))
        z.writestr('ppt/slides/_rels/slide1.xml.rels', rels)

HALF = len(URL) // 2
VARIANTS = {
    # the URL as plain text: the control
    'v1': (para(run(URL)), 't', SLIDE_RELS_PLAIN),
    # the same URL as one hyperlinked run
    'v2': (para(run(URL, 'rId9')), 't', SLIDE_RELS_LINK),
    # the link starting part way along a line
    'v3': (para(run('See ') + run(URL, 'rId9')), 't', SLIDE_RELS_LINK),
    # v2 middle-anchored, and its plain-text control
    'v4': (para(run(URL, 'rId9')), 'ctr', SLIDE_RELS_LINK),
    'v5': (para(run(URL)), 'ctr', SLIDE_RELS_PLAIN),
    # v2 bottom-anchored
    'v6': (para(run(URL, 'rId9')), 'b', SLIDE_RELS_LINK),
    # a link short enough to fit
    'v7': (para(run(SHORT, 'rId9')), 't', SLIDE_RELS_LINK),
    # the SAME link split across two a:r -- two fields in OOXML where ODF has one
    'v8': (para(run(URL[:HALF], 'rId9') + run(URL[HALF:], 'rId9')), 't', SLIDE_RELS_LINK),
    # text after the field
    'v9': (para(run(URL, 'rId9') + run(' and more text after it')), 't', SLIDE_RELS_LINK),
    # an a:hlinkClick that sets no property at all: r:id="" and nothing else
    'v10': (para(run(URL, '')), 't', SLIDE_RELS_PLAIN),
    # an a:hlinkClick with an empty r:id but a tooltip: one property, so a field
    'v11': (para(f'<a:r><a:rPr lang="en-US" sz="1600"><a:latin typeface="Liberation Sans"/>'
                 f'<a:hlinkClick xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" r:id="" tooltip="t"/>'
                 f'</a:rPr><a:t>{URL}</a:t></a:r>'), 't', SLIDE_RELS_PLAIN),
    # a centred paragraph holding the field
    'v12': (f'<a:p><a:pPr algn="ctr"><a:defRPr sz="1600"/></a:pPr>{run(URL, "rId9")}</a:p>',
            't', SLIDE_RELS_LINK),
    # two paragraphs: a field paragraph followed by an ordinary one, to read where
    # the paragraph after a spill starts
    'v13': (para(run(URL, 'rId9')) + para(run('AFTER')), 't', SLIDE_RELS_LINK),
    'v14': (para(run(URL)) + para(run('AFTER')), 't', SLIDE_RELS_PLAIN),
}

for name, (body, anchor, rels) in VARIANTS.items():
    deck(OUT / f'{name}.pptx', body, anchor, rels)
print(f'{len(VARIANTS)} decks in {OUT}')
