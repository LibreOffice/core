#!/usr/bin/env python3
"""Fixture 4 -- measure each renderer's SUPERSCRIPT advance through a channel that is not quantised.

A cell holds `i ` at 8 pt followed by a superscript run of k copies of `19`. Sweeping the stated
cell width one twip at a time gives the narrowest cell that keeps it on one line, W(k). The base
term and the cell margins are identical in every arm, so the SLOPE of W against k is the advance
of one `19` at the superscript's own size, in twips, with every fixed term cancelled -- the same
trick CLAUDE.md prescribes for advance comparison. With k up to 16 the slope is resolved to
1/15 twip = 0.003 pt.
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

KS = [1, 2, 3, 4, 6, 8, 12, 16]
BASE_HP = 16            # 8 pt


def arm(name, tw, k):
    rpr = f'<w:rPr><w:rFonts w:cs="Arial"/><w:sz w:val="{BASE_HP}"/><w:szCs w:val="{BASE_HP}"/></w:rPr>'
    rpr2 = (f'<w:rPr><w:rFonts w:cs="Arial"/><w:sz w:val="{BASE_HP}"/><w:szCs w:val="{BASE_HP}"/>'
            '<w:vertAlign w:val="superscript"/></w:rPr>')
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        f'<w:tbl><w:tblPr><w:tblW w:w="{tw}" w:type="dxa"/><w:jc w:val="left"/>'
        '<w:tblLayout w:type="fixed"/><w:tblCellMar><w:top w:w="0" w:type="dxa"/>'
        '<w:left w:w="108" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/>'
        '<w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        f'<w:tblGrid><w:gridCol w:w="{tw}"/></w:tblGrid>'
        f'<w:tr><w:tc><w:tcPr><w:tcW w:w="{tw}" w:type="dxa"/>'
        '<w:tcBorders><w:top w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
        '<w:bottom w:val="single" w:sz="8" w:space="0" w:color="auto"/></w:tcBorders>'
        '</w:tcPr><w:p><w:pPr><w:jc w:val="left"/></w:pPr>'
        f'<w:r>{rpr}<w:t xml:space="preserve">i </w:t></w:r>'
        f'<w:r>{rpr2}<w:t>{"19" * k}</w:t></w:r>'
        '</w:p></w:tc></w:tr></w:tbl>')


base_tw = face.twips('i ', 8.0)
body, meta = [], []
for k in KS:
    # centre the sweep on the widest plausible model (superscript at 0.58 x 8 pt)
    mid = base_tw + k * face.twips('19', 4.65)
    lo, hi = int(mid) + 216 - 6, int(mid) + 216 + 4
    meta.append((k, base_tw, lo, hi))
    for tw in range(lo, hi + 1):
        body.append(arm(f'P{k}_{tw}', tw, k))

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
print(f'{sys.argv[1]}: {len(body)} arms;  base "i " = {base_tw:.4f} tw;  "19" at 4.60 = '
      f'{face.twips("19", 4.60):.4f} tw, at 4.64 = {face.twips("19", 4.64):.4f}, at 4.65 = {face.twips("19", 4.65):.4f}')
