#!/usr/bin/env python3
"""A vertical merge that CROSSES A PAGE BOUNDARY, which is the attribute round 156's arms never varied.

Round 156 gave a covered cell the merge master's borders and one corpus `.doc` of its four movers
worsened, on pages 2 and 3 while page 1 improved. Its tables cross pages, and a merge whose covered
rows fall on the next page is the one shape none of that round's fixtures held: the master is then on
the page before, and the part placed on the new page has to charge its own boundary.

Each arm is a spacer paragraph of N empty lines followed by a two-column table whose left column is
merged over all six rows, so the merge straddles the break at a different row per arm. The right
column's cells are numbered, so which row landed where is read off the page rather than inferred.

    make-split.py <out.docx>
"""
import sys, zipfile
src = open('../faa-r152/make-fixture.py').read().split("out = sys.argv[1]")[0]
ns = {}
exec(compile(src, 'make-fixture.py', 'exec'), ns)
CT, RELS, DRELS, STYLES = ns['CT'], ns['RELS'], ns['DRELS'], ns['STYLES']
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

single = lambda side, sz: f'<w:{side} w:val="single" w:sz="{sz}" w:space="0" w:color="auto"/>'
NIL = '<w:{0} w:val="nil"/>'
SIDES = single('left', 8) + single('right', 8)
MASTER = single('top', 24) + single('bottom', 24) + SIDES
PLAIN = NIL.format('top') + NIL.format('bottom') + SIDES
LAST = NIL.format('top') + single('bottom', 8) + SIDES


def cell(borders, vmerge=None, text='x'):
    vm = '' if vmerge is None else (f'<w:vMerge w:val="{vmerge}"/>' if vmerge else '<w:vMerge/>')
    return (f'<w:tc><w:tcPr><w:tcW w:w="2000" w:type="dxa"/>{vm}<w:tcBorders>{borders}</w:tcBorders></w:tcPr>'
            f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
            f'<w:t>{text}</w:t></w:r></w:p></w:tc>')


def arm(name, spacer, merged):
    blank = ('<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
             '<w:t>.</w:t></w:r></w:p>') * spacer
    rows = []
    for i in range(6):
        borders = MASTER if i == 0 else (LAST if i == 5 else PLAIN)
        left = cell(borders, ('restart' if i == 0 else '') if merged else None, f'L{i}')
        right = cell(borders, None, f'{name}{i}')
        rows.append(f'<w:tr><w:trPr><w:trHeight w:val="450"/></w:trPr>{left}{right}</w:tr>')
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>' + blank +
        '<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/><w:jc w:val="left"/><w:tblLayout w:type="fixed"/>'
        '<w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/>'
        '<w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="2000"/></w:tblGrid>'
        + ''.join(rows) + '</w:tbl>')


# One arm per page-break position: the spacer decides which row of the six the break falls in.
ARMS = [('M', 62, True), ('N', 64, True), ('P', 66, True),
        ('Q', 62, False), ('R', 64, False), ('S', 66, False)]

body = []
for i, (name, spacer, merged) in enumerate(ARMS):
    body.append(('<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:br w:type="page"/></w:r></w:p>' if i else '')
                + arm(name, spacer, merged))

doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:document xmlns:w="{W}"><w:body>'
       + ''.join(body)
       + '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
         '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0" w:gutter="0"/>'
         '</w:sectPr></w:body></w:document>')
with zipfile.ZipFile(sys.argv[1], 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/_rels/document.xml.rels', DRELS)
    z.writestr('word/styles.xml', STYLES)
    z.writestr('word/document.xml', doc)
print(f'{sys.argv[1]}: {len(ARMS)} arms')
