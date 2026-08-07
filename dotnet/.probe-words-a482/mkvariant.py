#!/usr/bin/env python3
"""Copy a DOCX with word/settings.xml edited by a simple string replacement."""
import sys, zipfile, shutil

src, dst, old, new = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
zin = zipfile.ZipFile(src)
zout = zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED)
for item in zin.infolist():
    b = zin.read(item.filename)
    if item.filename == 'word/settings.xml':
        s = b.decode('utf8')
        if old not in s:
            print(f'!! {old!r} not present', file=sys.stderr)
        s = s.replace(old, new)
        b = s.encode('utf8')
    zout.writestr(item, b)
zout.close()
print(dst)
