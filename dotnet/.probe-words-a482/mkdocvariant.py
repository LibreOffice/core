#!/usr/bin/env python3
"""Copy a DOCX with word/document.xml edited by a string replacement."""
import sys, zipfile

src, dst, old, new = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
zin = zipfile.ZipFile(src)
zout = zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED)
n = 0
for item in zin.infolist():
    b = zin.read(item.filename)
    if item.filename == 'word/document.xml':
        s = b.decode('utf8')
        n = s.count(old)
        b = s.replace(old, new).encode('utf8')
    zout.writestr(item, b)
zout.close()
print(f'{dst}: {n} replacements')
