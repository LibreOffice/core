#!/usr/bin/env python3
"""Copy a DOCX with every `v:line` inside an `mc:Fallback` deleted, nothing else changed.

If the reference's rendering is unchanged by that edit, the fallback's VML lines take no
ink at all in 26.2.4.2 and the reach of reading them is nil for that document.
"""
import sys, zipfile, re, pathlib
from xml.etree import ElementTree as ET

VML='urn:schemas-microsoft-com:vml'; MC='http://schemas.openxmlformats.org/markup-compatibility/2006'
src, dst = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
zin = zipfile.ZipFile(src); removed = 0
with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zout:
    for item in zin.infolist():
        data = zin.read(item.filename)
        if item.filename.startswith('word/') and item.filename.endswith('.xml'):
            try:
                for pfx, uri in (('v', VML), ('mc', MC),
                                 ('w','http://schemas.openxmlformats.org/wordprocessingml/2006/main'),
                                 ('r','http://schemas.openxmlformats.org/officeDocument/2006/relationships'),
                                 ('o','urn:schemas-microsoft-com:office:office'),
                                 ('w10','urn:schemas-microsoft-com:office:word'),
                                 ('wp','http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing'),
                                 ('a','http://schemas.openxmlformats.org/drawingml/2006/main')):
                    ET.register_namespace(pfx, uri)
                tree = ET.fromstring(data)
            except Exception:
                zout.writestr(item, data); continue
            parent = {c: p for p in tree.iter() for c in p}
            victims = []
            for fb in tree.iter(f'{{{MC}}}Fallback'):
                for ln in fb.iter(f'{{{VML}}}line'):
                    victims.append(ln)
            for v in victims:
                parent[v].remove(v); removed += 1
            data = ET.tostring(tree, encoding='UTF-8', xml_declaration=True)
        zout.writestr(item, data)
print(f'{removed} v:line removed -> {dst}')
