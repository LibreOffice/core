#!/usr/bin/env python3
"""Author a gradient-path probe deck from a corpus deck.

Keeps every part of the source package and rewrites slide N's <p:bg> to a
path gradient with a stated a:fillToRect, stripping the slide's own shapes so
nothing but the background is drawn.  The point is to ask the installed
soffice what gradient style/angle/centre it derives for each fillToRect, by
reading its own flat-ODF export, rather than inferring it from the C++ in the
tree (which is a development branch, not the reference binary).
"""
import re, shutil, sys, zipfile

SRC = sys.argv[1]
DST = sys.argv[2]

# (path, fillToRect attribute string).  Two colours far apart so the ramp
# direction is unmistakable in the render.
CASES = [
    ('circle', 'l="50000" t="50000" r="50000" b="50000"'),   # centre
    ('circle', 'l="0" t="0" r="0" b="0"'),                   # whole rect -> centre
    ('circle', 'l="100000" t="100000"'),                     # bottom-right corner
    ('circle', 'l="0" t="0" r="100000" b="100000"'),         # top-left corner
    ('circle', 'l="100000" t="0" r="0" b="100000"'),         # top-right corner
    ('circle', 'l="0" t="100000" r="100000" b="0"'),         # bottom-left corner
    ('circle', 'l="50000" t="0" r="50000" b="100000"'),      # top centre
    ('circle', 'l="50000" t="100000" r="50000" b="0"'),      # bottom centre
    ('circle', 'l="0" t="50000" r="100000" b="50000"'),      # left centre
    ('circle', 'l="100000" t="50000" r="0" b="50000"'),      # right centre
    ('circle', 'l="50000" t="-80000" r="50000" b="180000"'), # the Office theme one
    ('circle', 'l="25000" t="25000" r="75000" b="75000"'),   # 25,25
    ('circle', 'l="75000" t="25000" r="25000" b="75000"'),   # 75,25
    ('circle', 'l="60000" t="50000" r="40000" b="50000"'),   # 60,50 - just off centre
    ('rect',   'l="100000" t="100000"'),                     # rect path, corner
    ('rect',   'l="50000" t="50000" r="50000" b="50000"'),   # rect path, centre
    ('shape',  'l="50000" t="50000" r="50000" b="50000"'),   # shape path
    ('circle', 'l="50000" t="130000" r="50000" b="-30000"'), # seen in the corpus
]

BG = ('<p:bg><p:bgPr><a:gradFill flip="none" rotWithShape="1"><a:gsLst>'
      '<a:gs pos="0"><a:srgbClr val="FF0000"/></a:gs>'
      '<a:gs pos="100000"><a:srgbClr val="0000FF"/></a:gs>'
      '</a:gsLst><a:path path="{p}"><a:fillToRect {r}/></a:path>'
      '</a:gradFill><a:effectLst/></p:bgPr></p:bg>')

zin = zipfile.ZipFile(SRC)
names = zin.namelist()
slides = sorted((n for n in names if re.fullmatch(r'ppt/slides/slide\d+\.xml', n)),
                key=lambda n: int(re.search(r'(\d+)', n.rsplit('/', 1)[1]).group(1)))
assert len(slides) >= len(CASES), f'{len(slides)} slides < {len(CASES)} cases'

with zipfile.ZipFile(DST, 'w', zipfile.ZIP_DEFLATED) as zout:
    for n in names:
        data = zin.read(n)
        if n in slides:
            i = slides.index(n)
            d = data.decode('utf8')
            d = re.sub(r'<p:bg>.*?</p:bg>', '', d, flags=re.S)
            if i < len(CASES):
                bg = BG.format(p=CASES[i][0], r=CASES[i][1])
            else:
                bg = ''
            d = d.replace('<p:cSld>', '<p:cSld>' + bg, 1)
            # strip every shape from the tree, leaving the group properties
            d = re.sub(r'(</p:grpSpPr>).*?(</p:spTree>)', r'\1\2', d, flags=re.S)
            data = d.encode('utf8')
        zout.writestr(n, data)
print(f'{DST}: {len(CASES)} cases over {len(slides)} slides')
for i, (p, r) in enumerate(CASES, 1):
    print(f'  slide {i:2d}  path={p:6s} {r}')
