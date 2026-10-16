#!/usr/bin/env python3
"""An authored .docx that asks 26.2.4.2 how a table's SHARED horizontal edge is decided.

    make-borderprobe.py <out.docx>

Every arm is a table of identical cells; only the two `w:tcBorders` facing each other across
one row boundary change. Two questions are asked at once and they are not the same question:

  * WHERE is the line drawn, and how far does it run -- the O82 question;
  * HOW TALL is the row, which is the same rule seen through `BorderHeight` and is O78's.

The page-break arms are the discriminator between "the shared edge is resolved once, so the
lower row's top border IS the upper row's bottom" and "a table's continuation page simply
repeats the table's own outer top border". Arm `pbNIL` states a table top border and nils
both sides of the split boundary: the first explanation draws nothing there, the second draws
a whole line.
"""
import sys
import zipfile

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')


def border(side, spec):
    if spec is None:
        return ''
    if spec == 'nil':
        return f'<w:{side} w:val="nil"/>'
    return f'<w:{side} w:val="single" w:sz="{spec}" w:space="0" w:color="000000"/>'


def cell(width, text, top, bottom, left='4', right='4'):
    borders = (border('top', top) + border('left', left)
               + border('bottom', bottom) + border('right', right))
    return (f'<w:tc><w:tcPr><w:tcW w:w="{width}" w:type="dxa"/>'
            f'<w:tcBorders>{borders}</w:tcBorders></w:tcPr>'
            f'<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr>'
            f'<w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
            f'<w:sz w:val="20"/></w:rPr><w:t>{text}</w:t></w:r></w:p></w:tc>')


def table(name, rows, cols=2, width=4800):
    grid = ''.join(f'<w:gridCol w:w="{width // cols}"/>' for _ in range(cols))
    body = ''
    for i, spec in enumerate(rows):
        cells = ''.join(cell(width // cols, f'{name}{i}{c}', spec[c][0], spec[c][1])
                        for c in range(cols))
        body += f'<w:tr><w:trPr><w:cantSplit/></w:trPr>{cells}</w:tr>'
    return (f'<w:p><w:pPr><w:spacing w:before="0" w:after="0"/></w:pPr>'
            f'<w:r><w:rPr><w:sz w:val="20"/></w:rPr><w:t>{name}</w:t></w:r></w:p>'
            f'<w:tbl><w:tblPr><w:tblW w:w="{width}" w:type="dxa"/>'
            f'<w:tblLayout w:type="fixed"/></w:tblPr>'
            f'<w:tblGrid>{grid}</w:tblGrid>{body}</w:tbl>')


def pair(top_of_second, bottom_of_first, cols=2):
    """Two rows: the first states `bottom_of_first`, the second `top_of_second`."""
    return [[('8', bottom_of_first)] * cols, [(top_of_second, '8')] * cols]


arms = []
# --- where is the line, and how tall is the row
arms.append(table('BB', pair('8', '8')))          # both state it -- the control
arms.append(table('BN', pair('nil', '8')))        # only the upper states it
arms.append(table('NT', pair('8', 'nil')))        # only the lower states it
arms.append(table('NN', pair('nil', 'nil')))      # neither
arms.append(table('WB', pair('4', '24')))         # 0.5 pt below a 3 pt -- which width wins
arms.append(table('WT', pair('24', '4')))         # and the other way round
# --- one row where the cells disagree: is the line whole or in pieces
arms.append(table('MIX', [[('8', '8'), ('8', '8')], [('nil', '8'), ('8', '8')]]))
arms.append(table('MIXN', [[('8', 'nil'), ('8', '8')], [('nil', '8'), ('8', '8')]]))

# --- the page-break discriminator: 44 rows, so the table splits, with the boundary nilled
long_rows = []
for i in range(44):
    top = 'nil' if i else '8'
    long_rows.append([(top, '8'), ('8', '8')])
arms.append(table('PBNIL', long_rows))

# The real discriminator: every interior edge is nil on BOTH sides, so a resolution model draws
# nothing at the split while a "repeat the table's own top border" model draws a whole line.
nn_rows = []
for i in range(90):
    nn_rows.append([('8' if i == 0 else 'nil', '8' if i == 89 else 'nil')] * 2)
arms.append(table('PBNN', nn_rows))

body = ''.join(arms)
doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       f'<w:document {NS}><w:body>{body}'
       f'<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
       f'<w:pgMar w:top="1417" w:right="1417" w:bottom="1417" w:left="1417"'
       f' w:header="708" w:footer="708" w:gutter="0"/></w:sectPr></w:body></w:document>')

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.document.main+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
        'relationships/officeDocument" Target="word/document.xml"/></Relationships>')

with zipfile.ZipFile(sys.argv[1], 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/document.xml', doc)
print(sys.argv[1])
