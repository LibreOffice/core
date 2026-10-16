#!/usr/bin/env python3
"""Build a one-attribute sweep fixture for O78's term (2): where does `Not Available19` stop fitting?

One table per arm, each arm a single 1x1 table whose ONLY difference from its neighbours is the
stated cell width in twips (or, in the variant arms, one named attribute). Every arm carries a
label paragraph naming it, so nothing is attributed by a y-range guess.

The cell content is copied from the real one: FAA 2025-26 Holdover Tables.docx, body table 1225,
row 7, cell 8 -- `w:jc center`, `w:sz 16` (8 pt) Arial, then a `w:vertAlign superscript` run `19`.
Table cell margins are Word's 108-twip default, stated explicitly so the arm does not depend on a
style.

    make-fixture.py <out.docx> [lo hi step]
"""
import sys, zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>'''

STYLES = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="{W}">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Times New Roman" w:eastAsia="Times New Roman" w:hAnsi="Times New Roman" w:cs="Times New Roman"/><w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:pPr><w:jc w:val="both"/></w:pPr><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:szCs w:val="22"/><w:lang w:val="en-CA"/></w:rPr></w:style>
<w:style w:type="table" w:default="1" w:styleId="TableNormal"><w:name w:val="Normal Table"/><w:tblPr><w:tblInd w:w="0" w:type="dxa"/><w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr></w:style>
</w:styles>'''


def runs(kind):
    """The cell's two runs. `kind` picks the one attribute under test."""
    base = ('<w:rPr><w:rFonts w:cs="Arial"/><w:color w:val="000000"/>'
            '<w:sz w:val="16"/><w:szCs w:val="16"/>{extra}<w:lang w:eastAsia="en-CA"/></w:rPr>')
    r1 = f'<w:r>{base.format(extra="")}<w:t xml:space="preserve">Not Available</w:t></w:r>'
    if kind == 'super':
        extra = '<w:vertAlign w:val="superscript"/>'
        r2 = f'<w:r>{base.format(extra=extra)}<w:t>19</w:t></w:r>'
    elif kind == 'plain':
        r2 = f'<w:r>{base.format(extra="")}<w:t>19</w:t></w:r>'
    elif kind.startswith('sz'):                      # explicit half-points, no escapement
        hp = kind[2:]
        r2 = ('<w:r><w:rPr><w:rFonts w:cs="Arial"/><w:color w:val="000000"/>'
              f'<w:sz w:val="{hp}"/><w:szCs w:val="{hp}"/><w:lang w:eastAsia="en-CA"/></w:rPr>'
              '<w:t>19</w:t></w:r>')
    elif kind == 'nosuffix':
        r2 = ''
    else:
        raise SystemExit('kind? ' + kind)
    return r1 + r2


def arm(name, tw, kind='super', nowrap=True, pct=None):
    nw = '<w:noWrap/>' if nowrap else ''
    if pct is None:
        tblw = f'<w:tblW w:w="{tw}" w:type="dxa"/>'
        tcw = f'<w:tcW w:w="{tw}" w:type="dxa"/>'
        grid = f'<w:gridCol w:w="{tw}"/>'
    else:
        tblw = f'<w:tblW w:w="{pct[0]}" w:type="pct"/>'
        tcw = f'<w:tcW w:w="{pct[1]}" w:type="pct"/>'
        grid = f'<w:gridCol w:w="{tw}"/>'
    return (
        f'<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/>'
        f'<w:sz w:val="14"/></w:rPr><w:t xml:space="preserve">ARM {name}</w:t></w:r></w:p>'
        f'<w:tbl><w:tblPr>{tblw}<w:jc w:val="left"/><w:tblLayout w:type="fixed"/>'
        '<w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/>'
        '<w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar>'
        f'</w:tblPr><w:tblGrid>{grid}</w:tblGrid>'
        '<w:tr><w:trPr><w:trHeight w:val="227"/></w:trPr>'
        f'<w:tc><w:tcPr>{tcw}'
        '<w:tcBorders><w:top w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
        '<w:left w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
        '<w:bottom w:val="single" w:sz="8" w:space="0" w:color="auto"/>'
        '<w:right w:val="single" w:sz="8" w:space="0" w:color="000000"/></w:tcBorders>'
        f'{nw}<w:vAlign w:val="center"/></w:tcPr>'
        '<w:p><w:pPr><w:jc w:val="center"/><w:rPr><w:rFonts w:cs="Arial"/><w:sz w:val="18"/></w:rPr></w:pPr>'
        f'{runs(kind)}</w:p></w:tc></w:tr></w:tbl>')


out = sys.argv[1]
lo = int(sys.argv[2]) if len(sys.argv) > 2 else 1230
hi = int(sys.argv[3]) if len(sys.argv) > 3 else 1300
step = int(sys.argv[4]) if len(sys.argv) > 4 else 1

body = []
names = []
for tw in range(lo, hi + 1, step):
    n = f'W{tw}'
    names.append(n)
    body.append(arm(n, tw))
# one-attribute variants, all at one width: the real cell's own 1262 twips
for kind in ('super', 'plain', 'sz9', 'sz10', 'nosuffix'):
    n = f'K-{kind}'
    names.append(n); body.append(arm(n, 1262, kind=kind))
for nw in (True, False):
    n = f'NW-{int(nw)}'
    names.append(n); body.append(arm(n, 1262, nowrap=nw))

doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:document xmlns:w="{W}"><w:body>'
       + ''.join(body)
       + '<w:sectPr><w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/>'
         '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0" w:gutter="0"/>'
         '</w:sectPr></w:body></w:document>')

with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/_rels/document.xml.rels', DRELS)
    z.writestr('word/styles.xml', STYLES)
    z.writestr('word/document.xml', doc)
print(f'{out}: {len(names)} arms, widths {lo}..{hi} step {step}')
