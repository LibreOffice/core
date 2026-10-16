#!/usr/bin/env python3
"""Fixture 7 -- which cell's stated top does a `w:trHeight` row charge when its cells disagree?

Two columns. The lower row is on its `w:trHeight` 450 floor. Arms vary ONE thing: which of the
lower row's two cells states a 1 pt top and which states `nil`, and -- in the V arms -- whether
the cell that states the 1 pt top is a `w:vMerge` CONTINUATION, which is the shape the real
document's rows 2 and 3 have.

  M<l><r>   plain two-column row, lower-left top = <l>, lower-right top = <r>
  V<l><r>   the same, but the LEFT column is one cell vertically merged over both rows
"""
import sys, zipfile
src = open('make-fixture.py').read().split("out = sys.argv[1]")[0]
ns = {}
exec(compile(src, 'make-fixture.py', 'exec'), ns)
CT, RELS, DRELS, STYLES = ns['CT'], ns['RELS'], ns['DRELS'], ns['STYLES']
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
S = {'B': '<w:top w:val="single" w:sz="8" w:space="0" w:color="auto"/>',
     'N': '<w:top w:val="nil"/>', 'A': ''}
OUT = ('<w:left w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
       '<w:right w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
       '<w:bottom w:val="single" w:sz="8" w:space="0" w:color="auto"/>')
UP = ('<w:top w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
      '<w:bottom w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
      '<w:left w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
      '<w:right w:val="single" w:sz="8" w:space="0" w:color="auto"/>')


def cell(w, borders, vmerge=None, text='x'):
    vm = '' if vmerge is None else (f'<w:vMerge w:val="{vmerge}"/>' if vmerge else '<w:vMerge/>')
    return (f'<w:tc><w:tcPr><w:tcW w:w="{w}" w:type="dxa"/>{vm}<w:tcBorders>{borders}</w:tcBorders></w:tcPr>'
            f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
            f'<w:t>{text}</w:t></w:r></w:p></w:tc>')


def arm(name, left, right, vmerged):
    up_left = cell(2000, UP, 'restart' if vmerged else None, 'u')
    up_right = cell(2000, UP, None, 'u')
    lo_left = cell(2000, S[left] + OUT, '' if vmerged else None, 'l')
    lo_right = cell(2000, S[right] + OUT, None, 'l')
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        '<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/><w:jc w:val="left"/><w:tblLayout w:type="fixed"/>'
        '<w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/>'
        '<w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="2000"/></w:tblGrid>'
        f'<w:tr>{up_left}{up_right}</w:tr>'
        f'<w:tr><w:trPr><w:trHeight w:val="450"/></w:trPr>{lo_left}{lo_right}</w:tr>'
        '</w:tbl><w:p><w:pPr><w:jc w:val="left"/></w:pPr></w:p>')


body = []
for v, tag in ((False, 'M'), (True, 'V')):
    for l in 'BNA':
        for r in 'BNA':
            body.append(arm(f'{tag}{l}{r}', l, r, v))
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
