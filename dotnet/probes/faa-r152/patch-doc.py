#!/usr/bin/env python3
"""One-attribute variant of the real document: re-spell ONE superscript so both renderers agree.

The cell that decides O78's term (2) is body table 1225, row 7, cell 8 -- `Not Available` plus a
`w:vertAlign superscript` run `19`. Fixture 2's Z arm shows that when the same `19` is stated at
an explicit 4.5 pt with no escapement, the two renderers need the SAME cell width (1259 twips) and
both fit inside the real cell. Re-spelling only that run therefore removes the wrap from our side
without moving any geometry, and the page below it can be measured.

Nothing else in the document is touched: byte-for-byte the same zip but for that one `w:rPr`.
"""
import re, shutil, sys, zipfile

src, dst = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(src) as z:
    names = z.namelist()
    data = {n: z.read(n) for n in names}

doc = data['word/document.xml'].decode('utf-8')
NEEDLE = ('<w:r w:rsidRPr="005F083F"><w:rPr><w:rFonts w:eastAsia="Times New Roman" w:cs="Arial"/>'
          '<w:color w:val="000000"/><w:sz w:val="16"/><w:szCs w:val="16"/>'
          '<w:vertAlign w:val="superscript"/><w:lang w:eastAsia="en-CA"/></w:rPr><w:t>19</w:t></w:r>')
REPL = ('<w:r w:rsidRPr="005F083F"><w:rPr><w:rFonts w:eastAsia="Times New Roman" w:cs="Arial"/>'
        '<w:color w:val="000000"/><w:sz w:val="9"/><w:szCs w:val="9"/>'
        '<w:lang w:eastAsia="en-CA"/></w:rPr><w:t>19</w:t></w:r>')
n = doc.count(NEEDLE)
print(f'occurrences of the exact run: {n}')
doc = doc.replace(NEEDLE, REPL)
data['word/document.xml'] = doc.encode('utf-8')

with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as z:
    for nm in names:
        z.writestr(nm, data[nm])
print('wrote', dst)
