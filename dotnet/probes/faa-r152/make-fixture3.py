#!/usr/bin/env python3
"""Fixture 3 -- is the line-fit comparison made in exact units or in whole twips?

For seven strings whose exact hmtx width in twips has a fractional part, sweep the stated cell
width by one twip at a time and find the narrowest cell each renderer keeps the string on one
line in. A renderer that compares exactly needs ceil(width); one that rounds the measurement to
whole twips first needs round(width). The two differ only where the fraction is below 0.5, which
is why the strings are chosen on their fractions.
"""
import sys, zipfile
src = open('make-fixture.py').read().split("out = sys.argv[1]")[0]
ns = {}
exec(compile(src, 'make-fixture.py', 'exec'), ns)
CT, RELS, DRELS, STYLES = ns['CT'], ns['RELS'], ns['DRELS'], ns['STYLES']
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
sys.path.insert(0, '.')
from hmtx import Face
face = Face('/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf')

# chosen so the exact width's fractional twip is between 0.08 and 0.42 -- the only region in
# which `ceil` and `round` disagree and the experiment can discriminate
STRINGS = ['i Reference', 'm Mississippi', 'm Hamburgefonstiv', 'i Viscosity',
           'i Alternate', 'i Available', 'i Xylophone', 'a Temperature',
           'a Objective', 'a Kilometres', 'a Wavelength', 'a Quicksand']


def arm(name, tw, text):
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        f'<w:tbl><w:tblPr><w:tblW w:w="{tw}" w:type="dxa"/><w:jc w:val="left"/>'
        '<w:tblLayout w:type="fixed"/><w:tblCellMar><w:top w:w="0" w:type="dxa"/>'
        '<w:left w:w="108" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/>'
        '<w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        f'<w:tblGrid><w:gridCol w:w="{tw}"/></w:tblGrid>'
        '<w:tr><w:trPr><w:trHeight w:val="227"/></w:trPr>'
        f'<w:tc><w:tcPr><w:tcW w:w="{tw}" w:type="dxa"/>'
        '<w:tcBorders><w:top w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
        '<w:left w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
        '<w:bottom w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
        '<w:right w:val="single" w:sz="8" w:space="0" w:color="auto"/></w:tcBorders>'
        '<w:vAlign w:val="center"/></w:tcPr>'
        '<w:p><w:pPr><w:jc w:val="center"/></w:pPr>'
        '<w:r><w:rPr><w:rFonts w:cs="Arial"/><w:sz w:val="16"/><w:szCs w:val="16"/></w:rPr>'
        f'<w:t xml:space="preserve">{text}</w:t></w:r></w:p></w:tc></w:tr></w:tbl>')


body, meta = [], []
for si, s in enumerate(STRINGS):
    text = s        # already carries its leading one-letter word, so a break opportunity exists
    exact = face.twips(text, 8.0)
    lo = int(exact) + 216 - 3
    hi = int(exact) + 216 + 4
    meta.append((si, s, text, exact))
    for tw in range(lo, hi + 1):
        body.append(arm(f'R{si}_{tw}', tw, text))

doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:document xmlns:w="{W}"><w:body>'
       + ''.join(body)
       + '<w:sectPr><w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/>'
         '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0" w:gutter="0"/>'
         '</w:sectPr></w:body></w:document>')
out = sys.argv[1]
with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/_rels/document.xml.rels', DRELS)
    z.writestr('word/styles.xml', STYLES)
    z.writestr('word/document.xml', doc)
with open(sys.argv[2], 'w') as fh:
    fh.write('i\tstring\ttext\texact_tw\tceil_W\tround_W\n')
    for si, s, text, exact in meta:
        fh.write(f'{si}\t{s}\t{text}\t{exact:.4f}\t{216 + -(-exact // 1):.0f}\t{216 + round(exact):.0f}\n')
print(f'{out}: {len(body)} arms')
