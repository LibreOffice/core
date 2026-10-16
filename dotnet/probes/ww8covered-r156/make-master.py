#!/usr/bin/env python3
"""Does a WW8 covered cell take the MERGE MASTER's borders, or its own?

Round 154 left O105 as *"a `.doc`'s covered cell carries borders the file does not state"*, measured
as four arms where 26.2.4.2 charges a 1 pt top that no cell states. 26.2.4.2's own `--convert-to fodt`
of that `.doc` shows every covered cell carrying the MASTER's cell style. That is consistent with two
rules, and one attribute separates them:

  A   the master states a 3 pt top, the covered cell states `nil`     -> 3 pt means the master's
  B   the master states 1 pt, the covered cell states a 3 pt top      -> 1 pt means the master's
  C   the control: no merge, the lower cell states `nil` under a 3 pt  -> its own, always

    make-master.py <out.docx>
"""
import sys, zipfile
src = open('../faa-r152/make-fixture.py').read().split("out = sys.argv[1]")[0]
ns = {}
exec(compile(src, 'make-fixture.py', 'exec'), ns)
CT, RELS, DRELS, STYLES = ns['CT'], ns['RELS'], ns['DRELS'], ns['STYLES']
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

def single(side, sz):
    return f'<w:{side} w:val="single" w:sz="{sz}" w:space="0" w:color="auto"/>'

NIL = '<w:{0} w:val="nil"/>'
SIDES = lambda sz: single('left', sz) + single('right', sz)


def cell(borders, vmerge=None, text='x'):
    vm = '' if vmerge is None else (f'<w:vMerge w:val="{vmerge}"/>' if vmerge else '<w:vMerge/>')
    return (f'<w:tc><w:tcPr><w:tcW w:w="2000" w:type="dxa"/>{vm}<w:tcBorders>{borders}</w:tcBorders></w:tcPr>'
            f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
            f'<w:t>{text}</w:t></w:r></w:p></w:tc>')


def arm(name, master_sz, covered_top, merged):
    up = single('top', master_sz) + single('bottom', master_sz) + SIDES(master_sz)
    low = ((single('top', covered_top) if covered_top else NIL.format('top'))
           + single('bottom', 8) + SIDES(8))
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        '<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/><w:jc w:val="left"/><w:tblLayout w:type="fixed"/>'
        '<w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/>'
        '<w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="2000"/></w:tblGrid>'
        f'<w:tr>{cell(up, "restart" if merged else None, "u")}{cell(up, None, "u")}</w:tr>'
        f'<w:tr><w:trPr><w:trHeight w:val="450"/></w:trPr>{cell(low, "" if merged else None, "l")}'
        f'{cell(NIL.format("top") + single("bottom", 8) + SIDES(8), None, "l")}</w:tr>'
        '</w:tbl><w:p><w:pPr><w:jc w:val="left"/></w:pPr></w:p>')


arms = [arm('A', 24, None, True),      # master 3 pt, covered nil
        arm('B', 8, 24, True),         # master 1 pt, covered 3 pt
        arm('C', 24, None, False),     # no merge, lower states nil under a 3 pt
        arm('D', 8, 24, False)]        # no merge, lower states 3 pt under a 1 pt

# One arm per page, so a test keys on the page rather than on a y range.
body = []
for i, one in enumerate(arms):
    body.append(('<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:br w:type="page"/></w:r></w:p>' if i else '')
                + one)

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
