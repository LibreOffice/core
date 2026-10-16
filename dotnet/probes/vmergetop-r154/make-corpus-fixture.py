#!/usr/bin/env python3
"""Build the corpus fixture for the covered-cell rule: one arm per PAGE, so a test keys on the page.

Eight arms, in pairs that differ in ONE attribute -- whether the cell a vertical merge covers states
a rule or `nil`. Each pair is one of the four places the rule is charged:

  1/2  FLOOR   a `w:trHeight` floor raised by the covered cell's stated top     (probe fixture 7)
  3/4  BAND    the band above a row, which is charged and NOT drawn             (probe fixture 8)
  5/6  BELOW   the band at the boundary below it, which IS drawn                (probe fixture 9)
  7/8  OUTER   the band below the table's last row                              (probe fixture 10)

    make-corpus-fixture.py <out.docx>
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


def cell(borders, vmerge=None, text='x'):
    vm = '' if vmerge is None else (f'<w:vMerge w:val="{vmerge}"/>' if vmerge else '<w:vMerge/>')
    return (f'<w:tc><w:tcPr><w:tcW w:w="2000" w:type="dxa"/>{vm}<w:tcBorders>{borders}</w:tcBorders></w:tcPr>'
            f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:sz w:val="16"/></w:rPr>'
            f'<w:t>{text}</w:t></w:r></w:p></w:tc>')


def table(rows):
    return ('<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/><w:jc w:val="left"/>'
            '<w:tblLayout w:type="fixed"/><w:tblCellMar><w:top w:w="0" w:type="dxa"/>'
            '<w:left w:w="108" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/>'
            '<w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
            '<w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="2000"/></w:tblGrid>'
            + ''.join(rows) + '</w:tbl>')


def row(cells, height=None):
    trpr = f'<w:trPr><w:trHeight w:val="{height}"/></w:trPr>' if height else ''
    return f'<w:tr>{trpr}{"".join(cells)}</w:tr>'


ALL = SINGLE.format('top', 8) + SINGLE.format('bottom', 8) + SIDES
NOBOT = SINGLE.format('top', 8) + NIL.format('bottom') + SIDES


def floor_arm(covered_states_top):
    """Fixture 7's VBN / VNN: a 450 twip floor, and the covered cell alone states the 1 pt top."""
    top = SINGLE.format('top', 8) if covered_states_top else NIL.format('top')
    low_out = NIL.format('bottom') + SIDES + SINGLE.format('bottom', 8)
    return table([
        row([cell(ALL, 'restart', 'u'), cell(ALL, None, 'u')]),
        row([cell(top + SIDES + SINGLE.format('bottom', 8), '', 'l'),
             cell(NIL.format('top') + SIDES + SINGLE.format('bottom', 8), None, 'l')], height=450),
    ])


def band_arm(covered_states_top):
    """Fixture 8's VB / VN: nothing but the covered cell can put a rule at the shared boundary."""
    top = SINGLE.format('top', 16) if covered_states_top else NIL.format('top')
    return table([
        row([cell(NOBOT, 'restart', 'u'), cell(NOBOT, None, 'u')]),
        row([cell(top + NIL.format('bottom') + SIDES, '', 'l'),
             cell(NIL.format('top') + NIL.format('bottom') + SIDES, None, 'l')]),
    ])


def below_arm(covered_states_bottom):
    """Fixture 9's WB / WN: the covered cell is row 1 of three and states the bottom at the 1|2 edge."""
    bottom = SINGLE.format('bottom', 16) if covered_states_bottom else NIL.format('bottom')
    return table([
        row([cell(NOBOT, 'restart', 't'), cell(NOBOT, None, 'u')]),
        row([cell(NIL.format('top') + bottom + SIDES, '', 'x'),
             cell(NIL.format('top') + NIL.format('bottom') + SIDES, None, 'm')]),
        row([cell(NIL.format('top') + SINGLE.format('bottom', 8) + SIDES, None, 'y'),
             cell(NIL.format('top') + SINGLE.format('bottom', 8) + SIDES, None, 'n')]),
    ])


def outer_arm(covered_states_bottom):
    """Fixture 10's ZB / ZN: the covered cell is the table's last row and states its outer bottom."""
    bottom = SINGLE.format('bottom', 16) if covered_states_bottom else NIL.format('bottom')
    return table([
        row([cell(NOBOT, 'restart', 'u'), cell(NOBOT, None, 'u')]),
        row([cell(NIL.format('top') + bottom + SIDES, '', 'l'),
             cell(NIL.format('top') + NIL.format('bottom') + SIDES, None, 'l')]),
    ])


ARMS = [floor_arm(True), floor_arm(False), band_arm(True), band_arm(False),
        below_arm(True), below_arm(False), outer_arm(True), outer_arm(False)]

body = []
for i, arm in enumerate(ARMS):
    brk = '<w:r><w:br w:type="page"/></w:r>' if i else ''
    # The OUTER pair needs a mark BELOW the table, because the band under the last row is the one
    # charge that moves nothing inside it: the two arms differ only in where what follows sits.
    after = ('<w:r><w:rPr><w:sz w:val="16"/></w:rPr><w:t>a</w:t></w:r>'
             if ARMS[i] in (ARMS[6], ARMS[7]) and i >= 6 else '')
    body.append(f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr>{brk}</w:p>' + arm
                + f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr>{after}</w:p>')

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
print(f'{sys.argv[1]}: {len(ARMS)} arms, one per page')
