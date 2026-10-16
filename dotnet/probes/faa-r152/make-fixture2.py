#!/usr/bin/env python3
"""Fixture 2 -- split the tenth of a point between the base string and the superscript.

Three width sweeps over the same cell, differing in ONE attribute each:

  S-<w>   `Not Available` + `19` with `w:vertAlign superscript`   (the real cell)
  Z-<w>   `Not Available` + `19` at an explicit `w:sz` of 9 half-points (4.5 pt), no escapement
  N-<w>   `Not Available` alone

and a separate family that reads the drawn superscript SIZE at eight base sizes:

  E-<hp>  one `w:sz=<hp>` run plus a superscript run, at the widest cell so nothing wraps
"""
import sys, zipfile
sys.path.insert(0, '.')
import importlib.util
spec = importlib.util.spec_from_file_location('mf', 'make-fixture.py')

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
CT = open('/dev/null').read()

# reuse the first generator's parts by importing its source without running __main__
src = open('make-fixture.py').read().split("out = sys.argv[1]")[0]
ns = {}
exec(compile(src, 'make-fixture.py', 'exec'), ns)
arm, CT, RELS, DRELS, STYLES = ns['arm'], ns['CT'], ns['RELS'], ns['DRELS'], ns['STYLES']


def esc_arm(name, hp):
    """A cell wide enough never to wrap, holding <w:sz=hp> text plus a superscript run."""
    rpr = f'<w:rPr><w:rFonts w:cs="Arial"/><w:sz w:val="{hp}"/><w:szCs w:val="{hp}"/></w:rPr>'
    rpr2 = (f'<w:rPr><w:rFonts w:cs="Arial"/><w:sz w:val="{hp}"/><w:szCs w:val="{hp}"/>'
            '<w:vertAlign w:val="superscript"/></w:rPr>')
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        '<w:tbl><w:tblPr><w:tblW w:w="6000" w:type="dxa"/><w:jc w:val="left"/>'
        '<w:tblLayout w:type="fixed"/><w:tblCellMar><w:top w:w="0" w:type="dxa"/>'
        '<w:left w:w="108" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/>'
        '<w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="6000"/></w:tblGrid>'
        '<w:tr><w:tc><w:tcPr><w:tcW w:w="6000" w:type="dxa"/></w:tcPr>'
        '<w:p><w:pPr><w:jc w:val="left"/></w:pPr>'
        f'<w:r>{rpr}<w:t xml:space="preserve">Mx</w:t></w:r>'
        f'<w:r>{rpr2}<w:t>19</w:t></w:r>'
        '</w:p></w:tc></w:tr></w:tbl>')


body, names = [], []
for tag, kind in (('S', 'super'), ('Z', 'sz9'), ('N', 'nosuffix')):
    for tw in range(1245, 1281):
        n = f'{tag}{tw}'
        names.append(n); body.append(arm(n, tw, kind=kind))
for hp in (12, 14, 16, 18, 20, 22, 24, 26, 28, 30):
    n = f'E{hp}'
    names.append(n); body.append(esc_arm(n, hp))

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
print(f'{out}: {len(names)} arms')
