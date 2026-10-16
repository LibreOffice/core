#!/usr/bin/env python3
"""Both one-attribute variants at once, to simulate the two fixes this round names.

  (a) the `19` superscript run re-spelled at an explicit 4.5 pt  -- simulates the superscript-size fix
  (b) in body table 1225 rows 2 and 3, ONE cell whose stated top is `nil` becomes a 1 pt `single`
      -- simulates seeing the 1 pt top that the row's `w:vMerge` CONTINUATION cell already states.

(b) is chosen so it cannot move the reference: in both rows the facing edge above is already a 1 pt
`single`, so 26.2.4.2's resolved band at that column is 1 pt before and after. The reference render
is checked against the unpatched one to confirm that.
"""
import sys, xml.etree.ElementTree as ET, zipfile

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
NS = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
ET.register_namespace('w', NS)


def q(t): return W + t


src, dst = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(src) as z:
    names = z.namelist()
    data = {n: z.read(n) for n in names}

doc = data['word/document.xml'].decode('utf-8')
NEEDLE = ('<w:r w:rsidRPr="005F083F"><w:rPr><w:rFonts w:eastAsia="Times New Roman" w:cs="Arial"/>'
          '<w:color w:val="000000"/><w:sz w:val="16"/><w:szCs w:val="16"/>'
          '<w:vertAlign w:val="superscript"/><w:lang w:eastAsia="en-CA"/></w:rPr><w:t>19</w:t></w:r>')
REPL = NEEDLE.replace('<w:sz w:val="16"/><w:szCs w:val="16"/><w:vertAlign w:val="superscript"/>',
                      '<w:sz w:val="9"/><w:szCs w:val="9"/>')
print('(a) superscript runs re-spelled:', doc.count(NEEDLE))
doc = doc.replace(NEEDLE, REPL)

root = ET.fromstring(doc)
tbl = list(root.find(q('body')))[1225]
changed = 0
for ri, ci in ((2, 6), (3, 4)):
    tc = tbl.findall(q('tr'))[ri].findall(q('tc'))[ci]
    top = tc.find(q('tcPr')).find(q('tcBorders')).find(q('top'))
    assert top.get(q('val')) == 'nil', (ri, ci, top.get(q('val')))
    top.set(q('val'), 'single'); top.set(q('sz'), '8'); top.set(q('space'), '0'); top.set(q('color'), 'auto')
    changed += 1
print('(b) nil tops promoted to a 1 pt single:', changed)
data['word/document.xml'] = ET.tostring(root, encoding='utf-8', xml_declaration=True)

with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as z:
    for nm in names:
        z.writestr(nm, data[nm])
print('wrote', dst)
