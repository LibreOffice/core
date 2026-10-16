#!/usr/bin/env python3
"""Authored one-attribute .docx probes for O83/O84/O85.

    make-probes.py <outdir>

Writes:
  align.docx    -- O85: one row boundary whose cells state borders of DIFFERENT widths.
  height.docx   -- O84: which row pays for a shared horizontal edge, and how much.
  height-nosettings.docx -- height.docx with `word/settings.xml` REMOVED, as the control
                    for CLAUDE.md's "a hand-built DOCX must include a settings part".
  split.docx    -- O83: a table whose rows split across a page.

Every arm is one table on its own page, so the first grid line of every arm is at the same y
and the arms are directly comparable. Only the attribute under test changes between arms.
"""
import sys, os, zipfile

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
R = 'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
NS = W + ' ' + R

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.document.main+xml"/>'
      '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.settings+xml"/></Types>')
CT_NO = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
         '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
         '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
         '<Default Extension="xml" ContentType="application/xml"/>'
         '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
         'officedocument.wordprocessingml.document.main+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
        'relationships/officeDocument" Target="word/document.xml"/></Relationships>')
DRELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
         '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
         '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
         'relationships/settings" Target="settings.xml"/></Relationships>')
SETTINGS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:settings {W}><w:compat/></w:settings>')

SECT = ('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1417" w:right="1417" w:bottom="1417" w:left="1417"'
        ' w:header="708" w:footer="708" w:gutter="0"/></w:sectPr>')


def bd(side, spec):
    """spec: None -> attribute absent; 'nil'; or (val, sz) e.g. ('single', 4)."""
    if spec is None:
        return ''
    if spec == 'nil':
        return f'<w:{side} w:val="nil"/>'
    val, sz = spec
    return f'<w:{side} w:val="{val}" w:sz="{sz}" w:space="0" w:color="000000"/>'


def para(text):
    return ('<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr>'
            '<w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
            f'<w:sz w:val="20"/></w:rPr><w:t>{text}</w:t></w:r></w:p>')


def cell(width, text, top, bottom, left, right, tcpr_extra=''):
    b = bd('top', top) + bd('left', left) + bd('bottom', bottom) + bd('right', right)
    borders = f'<w:tcBorders>{b}</w:tcBorders>' if b else ''
    return (f'<w:tc><w:tcPr><w:tcW w:w="{width}" w:type="dxa"/>{borders}{tcpr_extra}</w:tcPr>'
            f'{para(text)}</w:tc>')


def tbl(name, rows, colw, tblborders='', trpr='<w:trPr><w:cantSplit/></w:trPr>'):
    """rows: list of list of (top, bottom, left, right) per cell."""
    grid = ''.join(f'<w:gridCol w:w="{w}"/>' for w in colw)
    body = ''
    for i, row in enumerate(rows):
        cs = ''.join(cell(colw[c], f'{name}{i}{c}', *row[c]) for c in range(len(row)))
        body += f'<w:tr>{trpr}{cs}</w:tr>'
    return (f'<w:tbl><w:tblPr><w:tblW w:w="{sum(colw)}" w:type="dxa"/>'
            f'<w:tblLayout w:type="fixed"/>{tblborders}</w:tblPr>'
            f'<w:tblGrid>{grid}</w:tblGrid>{body}</w:tbl>')


def page(name, content):
    """A label paragraph, the arm, then a hard page break."""
    return para(f'ARM {name}') + content + ('<w:p><w:pPr/><w:r><w:br w:type="page"/></w:r></w:p>')


def write(path, docbody, settings=True):
    doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document {NS}><w:body>{docbody}{SECT}</w:body></w:document>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT if settings else CT_NO)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', doc)
        if settings:
            z.writestr('word/_rels/document.xml.rels', DRELS)
            z.writestr('word/settings.xml', SETTINGS)
    print(path)


S = ('single', 4)     # 0.5 pt
M = ('single', 8)     # 1.0 pt
L = ('single', 12)    # 1.5 pt
X = ('single', 24)    # 3.0 pt
D = ('double', 12)    # 1.5 pt double: 0.5 / 0.5 / 0.5
NIL = 'nil'
OUT = ('single', 8)   # the frame of every probe table, so the arms are locatable

COL3 = [1600, 1600, 1600]
COL2 = [2400, 2400]


# ---------------------------------------------------------------- align.docx (O85)
arms = []
# A1: one boundary, three columns stating three DIFFERENT single widths.
#     Row 2 states nil on top everywhere, so exactly one side states each edge.
arms.append(page('A1', tbl('A1', [
    [(OUT, S, OUT, OUT), (OUT, X, OUT, OUT), (OUT, L, OUT, OUT)],
    [(NIL, OUT, OUT, OUT), (NIL, OUT, OUT, OUT), (NIL, OUT, OUT, OUT)],
], COL3)))
# A2: the same three widths stated by the LOWER row's TOP instead. One attribute moved.
arms.append(page('A2', tbl('A2', [
    [(OUT, NIL, OUT, OUT), (OUT, NIL, OUT, OUT), (OUT, NIL, OUT, OUT)],
    [(S, OUT, OUT, OUT), (X, OUT, OUT, OUT), (L, OUT, OUT, OUT)],
], COL3)))
# A3: O85's own shape -- a 1.5 pt DOUBLE beside a 0.5 pt single.
arms.append(page('A3', tbl('A3', [
    [(OUT, S, OUT, OUT), (OUT, D, OUT, OUT), (OUT, S, OUT, OUT)],
    [(NIL, OUT, OUT, OUT), (NIL, OUT, OUT, OUT), (NIL, OUT, OUT, OUT)],
], COL3)))
# A4: the double stated by the lower row's top instead (MirrorSelf's arm).
arms.append(page('A4', tbl('A4', [
    [(OUT, NIL, OUT, OUT), (OUT, NIL, OUT, OUT), (OUT, NIL, OUT, OUT)],
    [(S, OUT, OUT, OUT), (D, OUT, OUT, OUT), (S, OUT, OUT, OUT)],
], COL3)))
# A5: a single 3 pt edge alone in a 2-column table -- the cleanest offset measurement.
arms.append(page('A5', tbl('A5', [
    [(OUT, X, OUT, OUT), (OUT, X, OUT, OUT)],
    [(NIL, OUT, OUT, OUT), (NIL, OUT, OUT, OUT)],
], COL2)))
write(os.path.join(sys.argv[1], 'align.docx'), ''.join(arms))


# --------------------------------------------------------------- height.docx (O84)
# Three rows. Only the pair of attributes facing across boundary 1 (between row 0 and row 1)
# changes. Boundary 2 is nil/nil in every arm, so it is the internal control.
def h(top_of_1, bottom_of_0, tblb='', r0=None, r1=None):
    rows = [
        [(OUT, bottom_of_0, OUT, OUT)] * 2,
        [(top_of_1, NIL, OUT, OUT)] * 2,
        [(NIL, OUT, OUT, OUT)] * 2,
    ]
    return rows

arms = []
arms.append(page('H0',  tbl('H0',  h(NIL, NIL), COL2)))   # control: nothing at boundary 1
arms.append(page('HB',  tbl('HB',  h(NIL, X),   COL2)))   # upper states 3 pt, lower nil
arms.append(page('HT',  tbl('HT',  h(X,   NIL), COL2)))   # lower states 3 pt, upper nil
arms.append(page('HBT', tbl('HBT', h(X,   X),   COL2)))   # both state 3 pt
arms.append(page('HWB', tbl('HWB', h(S,   X),   COL2)))   # upper 3 pt vs lower 0.5 pt
arms.append(page('HWT', tbl('HWT', h(X,   S),   COL2)))   # upper 0.5 pt vs lower 3 pt
arms.append(page('HS',  tbl('HS',  h(S,   S),   COL2)))   # both 0.5 pt
# Table-level borders: the cell states nothing and `insideH` supplies the edge.
IH3 = '<w:tblBorders><w:insideH w:val="single" w:sz="24" w:space="0" w:color="000000"/></w:tblBorders>'
arms.append(page('TB0', tbl('TB0', [
    [(OUT, None, OUT, OUT)] * 2, [(None, None, OUT, OUT)] * 2, [(None, OUT, OUT, OUT)] * 2,
], COL2, IH3)))
# ... and the cell REPLACES it: an explicit nil on both facing sides against insideH 3 pt.
arms.append(page('TBN', tbl('TBN', [
    [(OUT, NIL, OUT, OUT)] * 2, [(NIL, NIL, OUT, OUT)] * 2, [(NIL, OUT, OUT, OUT)] * 2,
], COL2, IH3)))
# ... and the cell replaces it with a NARROWER stated border.
arms.append(page('TBS', tbl('TBS', [
    [(OUT, S, OUT, OUT)] * 2, [(S, NIL, OUT, OUT)] * 2, [(NIL, OUT, OUT, OUT)] * 2,
], COL2, IH3)))
body = ''.join(arms)
write(os.path.join(sys.argv[1], 'height.docx'), body)
write(os.path.join(sys.argv[1], 'height-nosettings.docx'), body, settings=False)


# ---------------------------------------------------------------- split.docx (O83)
# One table, every interior edge stated 1 pt on the UPPER side only, long enough to split.
# Row 30 holds 40 paragraphs so the row itself must straddle the page boundary.
rows = []
for i in range(46):
    rows.append([(NIL, M if i < 45 else OUT, OUT, OUT)] * 2)
grid = ''.join(f'<w:gridCol w:w="{w}"/>' for w in COL2)
body_rows = ''
for i, row in enumerate(rows):
    cells = []
    for c in range(2):
        top, bottom, left, right = row[c]
        b = bd('top', top) + bd('left', left) + bd('bottom', bottom) + bd('right', right)
        text = ''.join(para(f'SP{i}{c} line {k}') for k in range(30)) if i == 30 else para(f'SP{i}{c}')
        cells.append(f'<w:tc><w:tcPr><w:tcW w:w="{COL2[c]}" w:type="dxa"/>'
                     f'<w:tcBorders>{b}</w:tcBorders></w:tcPr>{text}</w:tc>')
    body_rows += '<w:tr>' + ''.join(cells) + '</w:tr>'
sp = (f'<w:tbl><w:tblPr><w:tblW w:w="{sum(COL2)}" w:type="dxa"/>'
      f'<w:tblLayout w:type="fixed"/></w:tblPr><w:tblGrid>{grid}</w:tblGrid>{body_rows}</w:tbl>')
write(os.path.join(sys.argv[1], 'split.docx'), para('ARM SP') + sp)
