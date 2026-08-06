#!/usr/bin/env python3
"""Give UG.CAO.00133's section index 1 (page 2) the same even+first *footer* shape its header has."""
import re, sys, zipfile, os

SRC = "/workspace/sample-files/words/batch-014/docx/UG.CAO.00133 Foreign Part 145 approvals - Language.docx"
OUT = sys.argv[1]
MODE = sys.argv[2]   # 'evenfirstftr' | 'defaultemptyftr'

z = zipfile.ZipFile(SRC)
doc = z.read('word/document.xml').decode('utf8')
secs = list(re.finditer(r'<w:sectPr\b.*?</w:sectPr>', doc, re.S))
t = secs[1]
s = t.group(0)
if MODE == 'evenfirstftr':
    add = '<w:footerReference w:type="even" r:id="rId15"/><w:footerReference w:type="first" r:id="rId17"/>'
else:
    add = '<w:footerReference w:type="default" r:id="rId15"/>'
s = s.replace('<w:pgSz', add + '<w:pgSz')
doc = doc[:t.start()] + s + doc[t.end():]
print("section 1 now:", s[:320])

os.makedirs(os.path.dirname(OUT), exist_ok=True)
with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED) as zo:
    for item in z.infolist():
        data = z.read(item.filename)
        if item.filename == 'word/document.xml':
            data = doc.encode('utf8')
        zo.writestr(item, data)
print("wrote", OUT)
