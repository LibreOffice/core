#!/usr/bin/env python3
"""Cuts an FAA holdover document's body down to one range, keeping every other part.

Used to bisect what in the document makes 26.2.4.2 discard a `Heading3` paragraph's direct
`w:pPr` (results.md SS2). Body[lo..hi] plus section 17's own `w:sectPr` is the whole body;
styles, numbering, theme, settings, headers and footers are carried over untouched, so the
only variable is which paragraphs are present.

Usage: toc-bisect.py <lo> <hi> <name>     # writes cut/<name>.docx
"""
import zipfile, os, sys
import xml.etree.ElementTree as ET
W='http://schemas.openxmlformats.org/wordprocessingml/2006/main'
src='/home/user/sample-files/words/pagination-001/docx/24-25_FAA_Holdover_Tables.docx'
lo,hi,name=int(sys.argv[1]),int(sys.argv[2]),sys.argv[3]
z=zipfile.ZipFile(src)
root=ET.fromstring(z.read('word/document.xml'))
body=root.find(f'{{{W}}}body')
keep=[body[i] for i in range(lo,hi+1)]
sect=body[1101].find(f'{{{W}}}pPr').find(f'{{{W}}}sectPr')
for ch in list(body): body.remove(ch)
for k in keep: body.append(k)
body.append(sect)
ET.register_namespace('w', W)
out=ET.tostring(root, encoding='utf-8', xml_declaration=True)
os.makedirs('cut',exist_ok=True)
with zipfile.ZipFile(f'cut/{name}.docx','w',zipfile.ZIP_DEFLATED) as o:
    for n in z.namelist():
        o.writestr(n, out if n=='word/document.xml' else z.read(n))
print(f'cut/{name}.docx')
