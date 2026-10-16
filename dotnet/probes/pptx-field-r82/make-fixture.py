#!/usr/bin/env python3
"""Write dotnet/tests/corpus/features/slide-hyperlink-field.pptx.

Six slides, one rule each, all in the geometry the variants were measured in: a 14 x 5 cm
box at (1 cm, 1 cm) with zero insets and no autofit, 16 pt Liberation Sans.  The figures the
tests assert are 26.2.4.2's own, read out of its PDF by `baselines.py`.
"""
import sys
sys.path.insert(0, __file__.rsplit('/', 1)[0])
import zipfile
from pathlib import Path
import importlib.util

spec = importlib.util.spec_from_file_location(
    'mk', Path(__file__).with_name('make-pptx-variants.py'))
mk = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mk)

OUT = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(
    '/home/user/wt-pptxfield/dotnet/tests/corpus/features/slide-hyperlink-field.pptx')

URL, SHORT = mk.URL, mk.SHORT
run, para, CM = mk.run, mk.para, mk.CM

SLIDES = [
    # 1  an over-long hyperlink run: a field, cell-broken and spilled at the ascent
    (para(run(URL, 'rId9')), 't'),
    # 2  the same characters as plain text: the control for both halves of the rule
    (para(run(URL)), 't'),
    # 3  the field in a middle-anchored box: the formatter counts its line once
    (para(run(URL, 'rId9')), 'ctr'),
    # 4  a link short enough to fit: nothing about it may move
    (para(run(SHORT, 'rId9')), 't'),
    # 5  <a:hlinkClick r:id=""/> and nothing else: the property map stays empty, so this is
    #    not a field, not blue and not underlined
    (para(run(URL, '')), 't'),
    # 6  a paragraph after a field: the painter's pen carries the spill, so it starts one
    #    full line height below the last spill line
    (para(run(URL, 'rId9')) + para(run('AFTER')), 't'),
]

def slide_xml(body, anchor, index):
    return mk.slide(body, anchor)

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>
<Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>
<Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>
<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>
""" + '\n'.join(
    f'<Override PartName="/ppt/slides/slide{i+1}.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>'
    for i in range(len(SLIDES))) + "\n</Types>"

PRES = ("""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:presentation xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/></p:sldMasterIdLst>
<p:sldIdLst>""" + ''.join(
    f'<p:sldId id="{256+i}" r:id="rId{10+i}"/>' for i in range(len(SLIDES)))
    + """</p:sldIdLst>
<p:sldSz cx="9144000" cy="6858000"/><p:notesSz cx="6858000" cy="9144000"/>
</p:presentation>""")

PRES_RELS = ("""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/>
""" + '\n'.join(
    f'<Relationship Id="rId{10+i}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" Target="slides/slide{i+1}.xml"/>'
    for i in range(len(SLIDES))) + "\n</Relationships>")

with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', mk.ROOT_RELS)
    z.writestr('ppt/presentation.xml', PRES)
    z.writestr('ppt/_rels/presentation.xml.rels', PRES_RELS)
    z.writestr('ppt/theme/theme1.xml', mk.theme())
    z.writestr('ppt/slideMasters/slideMaster1.xml', mk.MASTER)
    z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels', mk.MASTER_RELS)
    z.writestr('ppt/slideLayouts/slideLayout1.xml', mk.LAYOUT)
    z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels', mk.LAYOUT_RELS)
    for i, (body, anchor) in enumerate(SLIDES):
        z.writestr(f'ppt/slides/slide{i+1}.xml', mk.slide(body, anchor))
        # slide 5 states r:id="" and must NOT declare a hyperlink relationship
        rels = mk.SLIDE_RELS_PLAIN if i in (1, 4) else mk.SLIDE_RELS_LINK
        z.writestr(f'ppt/slides/_rels/slide{i+1}.xml.rels', rels)

print('wrote', OUT)
