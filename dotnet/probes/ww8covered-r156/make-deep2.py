#!/usr/bin/env python3
"""A merge spanning FOUR rows whose master states NO BOTTOM: only a covered cell can charge a row.

§2's arms all merge two rows, so the covered cell is always the merge's last. A corpus `.doc`
regressed by about 18 pt from page 2 onwards after the master's borders were carried, which is what
one point per covered row over a long merge would look like -- so the depth is the attribute to vary.

  A   a four-row merge, master states a 3 pt top, every covered cell states `nil`
  B   the same with no merge at all, every lower cell stating `nil`      (the control)

Every row is on a `w:trHeight` floor of 450 twips, so each row's height is the floor plus whatever
top rule that row is charged.

    make-deep.py <out.docx>
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


def cell(borders, vmerge=None, text='x'):
    vm = '' if vmerge is None else (f'<w:vMerge w:val="{vmerge}"/>' if vmerge else '<w:vMerge/>')
    return (f'<w:tc><w:tcPr><w:tcW w:w="2000" w:type="dxa"/>{vm}<w:tcBorders>{borders}</w:tcBorders></w:tcPr>'
            f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
            f'<w:t>{text}</w:t></w:r></w:p></w:tc>')


def arm(name, merged):
    master = single('top', 24) + NIL.format('bottom') + SIDES
    lower = NIL.format('top') + NIL.format('bottom') + SIDES
    last = NIL.format('top') + single('bottom', 8) + SIDES
    rows = [f'<w:tr><w:trPr><w:trHeight w:val="450"/></w:trPr>'
            f'{cell(master, "restart" if merged else None, "a")}{cell(master, None, "a")}</w:tr>']
    for i, text in enumerate('bcd'):
        borders = last if text == 'd' else lower
        rows.append(f'<w:tr><w:trPr><w:trHeight w:val="450"/></w:trPr>'
                    f'{cell(borders, "" if merged else None, text)}{cell(borders, None, text)}</w:tr>')
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        '<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/><w:jc w:val="left"/><w:tblLayout w:type="fixed"/>'
        '<w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/>'
        '<w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="2000"/></w:tblGrid>'
        + ''.join(rows) + '</w:tbl><w:p><w:pPr><w:jc w:val="left"/></w:pPr></w:p>')


body = []
for i, one in enumerate([arm('A', True), arm('B', False)]):
    body.append(('<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:br w:type="page"/></w:r></w:p>' if i else '') + one)

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
