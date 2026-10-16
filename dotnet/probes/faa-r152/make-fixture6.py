#!/usr/bin/env python3
"""Fixture 6 -- what border does a `w:trHeight` row charge: its OWN stated top, or the RESOLVED one?

Nine two-row arms. The upper row is content-driven; the lower row states `w:trHeight` 450 (22.5 pt),
well above its one line of content, so it is on the floor branch. The ONLY thing that changes
between arms is the pair (upper row's stated bottom, lower row's stated top), over
single 1 pt / `nil` / absent. The observable is the lower row's drawn height.

If the floor branch charges the row's own stated top, the three arms in a column (same lower top)
are equal. If it charges the resolved band, the three arms in a row (same resolved edge) are equal.
"""
import sys, zipfile
src = open('make-fixture.py').read().split("out = sys.argv[1]")[0]
ns = {}
exec(compile(src, 'make-fixture.py', 'exec'), ns)
CT, RELS, DRELS, STYLES = ns['CT'], ns['RELS'], ns['DRELS'], ns['STYLES']
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

SIDES = {'B': '<w:{s} w:val="single" w:sz="8" w:space="0" w:color="auto"/>',
         'N': '<w:{s} w:val="nil"/>',
         'A': ''}


def arm(name, upper_bottom, lower_top):
    ub = SIDES[upper_bottom].format(s='bottom')
    lt = SIDES[lower_top].format(s='top')
    outer = ('<w:left w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
             '<w:right w:val="single" w:sz="8" w:space="0" w:color="auto"/>')
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        '<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/><w:jc w:val="left"/>'
        '<w:tblLayout w:type="fixed"/><w:tblCellMar><w:top w:w="0" w:type="dxa"/>'
        '<w:left w:w="108" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/>'
        '<w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="4000"/></w:tblGrid>'
        '<w:tr><w:tc><w:tcPr><w:tcW w:w="4000" w:type="dxa"/><w:tcBorders>'
        '<w:top w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
        f'{ub}{outer}</w:tcBorders></w:tcPr>'
        '<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
        '<w:t>upper</w:t></w:r></w:p></w:tc></w:tr>'
        '<w:tr><w:trPr><w:trHeight w:val="450"/></w:trPr>'
        '<w:tc><w:tcPr><w:tcW w:w="4000" w:type="dxa"/><w:tcBorders>'
        f'{lt}<w:bottom w:val="single" w:sz="8" w:space="0" w:color="auto"/>{outer}</w:tcBorders></w:tcPr>'
        '<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
        '<w:t>lower</w:t></w:r></w:p></w:tc></w:tr></w:tbl>'
        '<w:p><w:pPr><w:jc w:val="left"/></w:pPr></w:p>')


body = [arm(f'{u}{l}', u, l) for u in 'BNA' for l in 'BNA']
doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:document xmlns:w="{W}"><w:body>'
       + ''.join(body)
       + '<w:sectPr><w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/>'
         '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0" w:gutter="0"/>'
         '</w:sectPr></w:body></w:document>')
with zipfile.ZipFile(sys.argv[1], 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/_rels/document.xml.rels', DRELS)
    z.writestr('word/styles.xml', STYLES)
    z.writestr('word/document.xml', doc)
print(f'{sys.argv[1]}: {len(body)} arms')
