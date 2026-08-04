#!/usr/bin/env python3
"""Rewrite one section's header references in UG.CAO.00133.

usage: ug133-probe3.py <out.docx> <section-index> <strip|point>
  strip  removes every w:headerReference from that section
  point  replaces them with a single default reference to header1 (which has content)
"""
import re, sys, zipfile, os

SRC = "/workspace/sample-files/words/batch-014/docx/UG.CAO.00133 Foreign Part 145 approvals - Language.docx"
OUT, IDX, MODE = sys.argv[1], int(sys.argv[2]), sys.argv[3]

z = zipfile.ZipFile(SRC)
doc = z.read('word/document.xml').decode('utf8')
secs = list(re.finditer(r'<w:sectPr\b.*?</w:sectPr>', doc, re.S))
t = secs[IDX]
s = re.sub(r'<w:headerReference[^/]*/>', '', t.group(0))
if MODE == 'point':
    s = s.replace('<w:pgSz', '<w:headerReference w:type="default" r:id="rId9"/><w:pgSz')
doc = doc[:t.start()] + s + doc[t.end():]
print(f"section {IDX} now:", s[:240])

os.makedirs(os.path.dirname(OUT), exist_ok=True)
with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED) as zo:
    for item in z.infolist():
        data = z.read(item.filename)
        if item.filename == 'word/document.xml':
            data = doc.encode('utf8')
        zo.writestr(item, data)
print("wrote", OUT)
