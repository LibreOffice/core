#!/usr/bin/env python3
"""Fixture 9 -- and does a continuation cell's stated BOTTOM rule feed the band below it?

Fixture 8 answers the top half. This is its mirror: three rows, the left column merged over rows 0
and 1, and the only statement of a rule at the 1|2 boundary is the CONTINUATION cell's own bottom.
Every other edge at that boundary is `nil`, so if the covered cell is not counted nothing is.

The observable is the baseline gap between row 1's and row 2's text, which is in the RIGHT column
because a covered cell draws no text of its own.

  Q<b>  no merge: row 1's left cell is an ordinary cell stating bottom = <b>   (the control)
  W<b>  the left column is merged over rows 0 and 1; row 1's left cell is its continuation

    make-bottom.py <out.docx>
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
ALLNIL = NIL.format('top') + NIL.format('bottom') + SIDES


def cell(borders, vmerge=None, text='x'):
    vm = '' if vmerge is None else (f'<w:vMerge w:val="{vmerge}"/>' if vmerge else '<w:vMerge/>')
    return (f'<w:tc><w:tcPr><w:tcW w:w="2000" w:type="dxa"/>{vm}<w:tcBorders>{borders}</w:tcBorders></w:tcPr>'
            f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
            f'<w:t>{text}</w:t></w:r></w:p></w:tc>')


def arm(name, bottom, vmerged):
    mid_left_borders = (NIL.format('top')
                        + (SINGLE.format('bottom', 16) if bottom == 'B' else NIL.format('bottom'))
                        + SIDES)
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        '<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/><w:jc w:val="left"/><w:tblLayout w:type="fixed"/>'
        '<w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/>'
        '<w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="2000"/></w:tblGrid>'
        f'<w:tr>{cell(SINGLE.format("top", 8) + NIL.format("bottom") + SIDES, "restart" if vmerged else None, "t")}'
        f'{cell(SINGLE.format("top", 8) + NIL.format("bottom") + SIDES, None, "u")}</w:tr>'
        f'<w:tr>{cell(mid_left_borders, "" if vmerged else None, "x")}{cell(ALLNIL, None, "m")}</w:tr>'
        f'<w:tr>{cell(NIL.format("top") + SINGLE.format("bottom", 8) + SIDES, None, "y")}'
        f'{cell(NIL.format("top") + SINGLE.format("bottom", 8) + SIDES, None, "n")}</w:tr>'
        '</w:tbl><w:p><w:pPr><w:jc w:val="left"/></w:pPr></w:p>')


body = [arm(f'{tag}{b}', b, v) for v, tag in ((False, 'Q'), (True, 'W')) for b in 'BN']
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
