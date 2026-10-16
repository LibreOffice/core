"""Does the sub-unity rule treat a paragraph's first line differently from its later ones?

Why this exists
---------------

`ascent-eighty-percent.py` gave every line its own `<w:p>`, so all eleven were
paragraph *first* lines and `IsParaLine()` was true for all of them.
`CalcRealHeight`'s shrink block is gated on exactly that, and its own comment
says "shrink first line of paragraph **too** on spacing < 100%" — so later lines
may take a different path and the probe could not have told.

This lays **one** paragraph that wraps to seven lines and reads every line's ink
top, at 50, 62.5, 75, 87.5 and 100 per cent.

What it found, 2026-08-15
-------------------------

**Two things, and the second is the more useful.**

First, the ascent rule holds for a wrapped paragraph as well. The first line's
ink shrink against the 100% page is 105, 78, 54 and 29 twips at 50, 62.5, 75 and
87.5 per cent, and `naturalAscent - (4 * H) / 5` with the *single-line* heights
126, 159, 189 and 220 predicts all four exactly. So the fix is not first-line
only, and the ink-top gaps within each paragraph are uniform, which says every
line in it shares the shrunken ascent rather than only the first.

Second, and this is a lead rather than a result: **the reference's line pitch is
not the same in a wrapped paragraph as in a paragraph per line.**

    w:line   pct      one para per line      wrapped paragraph      ours
       120   50.0                   126                    127       127
       150   62.5                   159                    160       157
       180   75.0                   189                    190       190
       210   87.5                   220              220 then 221    223
       240  100.0                   253                    253       253

Every sub-unity value is **one twip taller** when the lines are inside one
paragraph, and at 87.5% the split is visible within a single page: the first gap
is 220 — the single-line value — and every later gap is 221.

That matters for two reasons. It is `PROP_LINE_SPACING_SHRINKS_FIRST_LINE`
showing itself directly, which is the compatibility flag the shrink block is
gated on. And it means **the twenty-one point table in
`subunity-line-spacing.py` was measured on the one paragraph shape where that
block fires** — so the "+1 twip on 16 of 21" reading describes first lines, not
lines in general. Against the wrapped shape we are exact at 50% and 75%.

Any further attempt on the height question should measure both shapes, because
they are different quantities and the single-line one is the special case.

Usage
-----

    XRAY_OFFICE_CLI=... python3 first-line-versus-later.py /abs/workdir
"""
import os, re, subprocess, sys, zipfile

FACE, HP = 'Liberation Serif', 22
CT=('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
    '<Default Extension="xml" ContentType="application/xml"/><Override PartName="/word/document.xml" '
    'ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/></Types>')
RELS=('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
      '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>')
W='xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
LINES=[120,150,180,210,240]

out=sys.argv[1]; cli=os.environ['XRAY_OFFICE_CLI']
os.makedirs(out+'/ours',exist_ok=True)
run=f'<w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/><w:sz w:val="{HP}"/>'
words=' '.join('alpha bravo charlie delta echo foxtrot golf hotel india juliet'.split()*14)
body=''
for i,l in enumerate(LINES):
    brk='<w:pageBreakBefore/>' if i else ''
    body+=(f'<w:p><w:pPr>{brk}<w:spacing w:line="{l}" w:lineRule="auto" w:before="0" w:after="0"/>'
           f'<w:rPr>{run}</w:rPr></w:pPr><w:r><w:rPr>{run}</w:rPr><w:t>{words}</w:t></w:r></w:p>')
doc=(f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
     '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/><w:pgMar w:top="567" w:right="567" w:bottom="567" w:left="567"/>'
     '</w:sectPr></w:body></w:document>')
p=out+'/wrap.docx'
with zipfile.ZipFile(p,'w',zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml',CT); z.writestr('_rels/.rels',RELS); z.writestr('word/document.xml',doc)
subprocess.run(['soffice','--headless','--convert-to','pdf','--outdir',out,p],capture_output=True,check=True)
subprocess.run([cli,'render','--quiet','--outdir',out+'/ours',p],check=True)
ref,ours=out+'/wrap.pdf',out+'/ours/wrap.pdf'
for f in (ref,ours):
    if not os.path.isfile(f): raise SystemExit(f'{f} not written')

def tops(pdf,page):
    t=subprocess.run(['pdftotext','-bbox','-f',str(page),'-l',str(page),pdf,'-'],capture_output=True,text=True,check=True).stdout
    return sorted({round(float(m.group(1)),3) for m in re.finditer(r'yMin="([\d.]+)"',t)})

for page,l in enumerate(LINES,1):
    r,o=tops(ref,page),tops(ours,page)
    gr=[round((r[i+1]-r[i])*20,1) for i in range(len(r)-1)]
    go=[round((o[i+1]-o[i])*20,1) for i in range(len(o)-1)]
    print(f'w:line={l} ({l/240*100:.1f}%)  ref {len(r)} lines, first ink {r[0]:.3f}')
    print(f'   ref gaps tw: {gr[:9]}')
    print(f'  ours gaps tw: {go[:9]}')
