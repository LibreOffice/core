#!/usr/bin/env python3
"""One-attribute rewrites of 028_Unit_Circle_Chart_Optimized_Graph, for 26.2.4.2 to answer.

    ./variants.py <outdir>

Each variant changes exactly one thing in word/document.xml and leaves every other part of the
package byte-identical, which is what makes the reference's answer attributable. Render them with

    /opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir <d> <v>.docx

and read the chart frame out of the page with
`.claude/skills/render-comparison/scripts/pdf-ops.py dump <pdf> --only fill`: the chart's own
background is the one white fill spanning most of the page.
"""
import pathlib, re, sys, zipfile

SRC = ('/home/user/sample-files/words/chartset-010/docx/'
       '028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx')

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
doc = zipfile.ZipFile(SRC).read('word/document.xml').decode('utf-8')

at = doc.find('2229163')                       # the chart anchor's own vertical offset
posv = doc[doc.rfind('<wp:positionV', 0, at):doc.find('</wp:positionV>', at) + 15]
anchor = doc[doc.rfind('<wp:anchor', 0, at):doc.find('>', doc.rfind('<wp:anchor', 0, at)) + 1]
pgmar = re.search(r'<w:pgMar[^/]*/>', doc).group(0)
wrap = re.search(r'<wp:wrapTight.*?</wp:wrapTight>|<wp:wrapTight[^>]*/>',
                 doc[at:doc.find('</wp:anchor>', at)], re.S).group(0)

EDITS = {
    # Where a page-relative vertical offset counts from.
    'margin':   (posv, posv.replace('relativeFrom="page"', 'relativeFrom="margin"')),
    'off0':     (posv, posv.replace('2229163', '0')),
    'offp72':   (posv, posv.replace('2229163', '3143563')),
    'offm36':   (posv, posv.replace('2229163', '-457200')),
    'offm200':  (posv, posv.replace('2229163', '-2540000')),
    'top2880':  (pgmar, pgmar.replace('w:top="1440"', 'w:top="2880"')),
    'top0':     (pgmar, pgmar.replace('w:top="1440"', 'w:top="0"')),
    # Which anchor attribute switches the base.
    'behind0':  (anchor, anchor.replace('behindDoc="1"', 'behindDoc="0"')),
    'incell0':  (anchor, anchor.replace('layoutInCell="1"', 'layoutInCell="0"')),
    'wrapnone': (wrap, '<wp:wrapNone/>'),
}

OUT.mkdir(parents=True, exist_ok=True)
for name, (old, new) in EDITS.items():
    assert old in doc and old != new, name
    target = OUT / f'v-{name}.docx'
    src = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as out:
        for item in src.infolist():
            data = src.read(item.filename)
            if item.filename == 'word/document.xml':
                data = doc.replace(old, new, 1).encode('utf-8')
            out.writestr(item, data)
    print('wrote', target)
