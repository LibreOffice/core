"""How many corpus DOCX carry a `continuous` section break that names furniture of its own.

A continuous break does not start a page, so in LibreOffice it cannot change the page style —
and a running head is a property of the page style. Measured on
`hdss-bulletin-issue-285-25-june-2025.docx`: its section 2 is continuous and names
`header2.xml`, the reference draws that header on no page at all, and changing only
`<w:type w:val="continuous"/>` to a next-page break makes the reference draw it on 9.
"""
import pathlib
import xml.etree.ElementTree as ET
import zipfile

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
corpus = pathlib.Path('/home/user/sample-files')
hits = []
scanned = 0
for p in sorted(corpus.rglob('*')):
    if not p.is_file() or p.suffix.lower() not in ('.docx', '.docm', '.dotx'):
        continue
    scanned += 1
    try:
        root = ET.fromstring(zipfile.ZipFile(p).read('word/document.xml'))
    except Exception:
        continue
    n_cont = 0
    n_cont_furniture = 0
    for sect in root.iter(W + 'sectPr'):
        kind = sect.find(W + 'type')
        if kind is None or kind.get(W + 'val') != 'continuous':
            continue
        n_cont += 1
        if sect.findall(W + 'headerReference') or sect.findall(W + 'footerReference'):
            n_cont_furniture += 1
    if n_cont:
        hits.append((str(p.relative_to(corpus)), n_cont, n_cont_furniture))
print('docx-family scanned:', scanned)
print('with any continuous section break:', len(hits))
print('whose continuous section names a header or footer:', sum(1 for h in hits if h[2]))
for h in hits:
    if h[2]:
        print('  ', h)
