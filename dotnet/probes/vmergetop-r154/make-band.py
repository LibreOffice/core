#!/usr/bin/env python3
"""Fixture 8 -- does a `w:vMerge` CONTINUATION cell's stated top rule feed the drawn BAND too?

Round 152's fixture 7 answers only half the question: every one of its arms holds the upper row's
bottom at a 1 pt `single`, so the resolved band at that boundary is 1 pt in all eighteen and the
fixture says nothing about whether the covered cell contributes to it. Here the upper row states
`nil` at that boundary and the lower row's ORDINARY cell states `nil` too, so the ONLY statement of
a top rule is the covered cell's -- and the observables separate the two answers:

  the band  -- is a rule drawn at that boundary at all, and how thick;
  the height -- with no `w:trHeight`, the row's height is its content plus the band it pays for.

  P<l>   plain two-column row, lower-left top = <l>            (the control: no merge)
  V<l>   the left column is one cell merged over both rows, the lower-left is its continuation
  H suffix: the lower row carries `w:trHeight w:val="450"`, which is fixture 7's own case

    make-band.py <out.docx>
"""
import sys, zipfile
src = open('../faa-r152/make-fixture.py').read().split("out = sys.argv[1]")[0]
ns = {}
exec(compile(src, 'make-fixture.py', 'exec'), ns)
CT, RELS, DRELS, STYLES = ns['CT'], ns['RELS'], ns['DRELS'], ns['STYLES']
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

SINGLE = '<w:{0} w:val="single" w:sz="{1}" w:space="0" w:color="auto"/>'
NIL = '<w:{0} w:val="nil"/>'
SIDES = SINGLE.format('left', 8) + SINGLE.format('right', 8)

# The upper row states `nil` at the shared boundary, so nothing but the lower row can put a rule there.
UP = SINGLE.format('top', 8) + NIL.format('bottom') + SIDES
LOW_OUT = NIL.format('bottom') + SIDES


def cell(w, borders, vmerge=None, text='x'):
    vm = '' if vmerge is None else (f'<w:vMerge w:val="{vmerge}"/>' if vmerge else '<w:vMerge/>')
    return (f'<w:tc><w:tcPr><w:tcW w:w="{w}" w:type="dxa"/>{vm}<w:tcBorders>{borders}</w:tcBorders></w:tcPr>'
            f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
            f'<w:t>{text}</w:t></w:r></w:p></w:tc>')


def arm(name, left, vmerged, height):
    top = SINGLE.format('top', 16) if left == 'B' else NIL.format('top')   # 2 pt, so the band is unmistakable
    trpr = f'<w:trPr><w:trHeight w:val="450"/></w:trPr>' if height else ''
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        '<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/><w:jc w:val="left"/><w:tblLayout w:type="fixed"/>'
        '<w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/>'
        '<w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="2000"/></w:tblGrid>'
        f'<w:tr>{cell(2000, UP, "restart" if vmerged else None, "u")}{cell(2000, UP, None, "u")}</w:tr>'
        f'<w:tr>{trpr}{cell(2000, top + LOW_OUT, "" if vmerged else None, "l")}'
        f'{cell(2000, NIL.format("top") + LOW_OUT, None, "l")}</w:tr>'
        '</w:tbl><w:p><w:pPr><w:jc w:val="left"/></w:pPr></w:p>')


body = []
for height in (False, True):
    for v, tag in ((False, 'P'), (True, 'V')):
        for l in 'BN':
            body.append(arm(f'{tag}{l}{"H" if height else ""}', l, v, height))

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
