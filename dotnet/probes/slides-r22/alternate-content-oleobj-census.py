#!/usr/bin/env python3
"""Which slides carry an mc:AlternateContent Choice LibreOffice takes and we do not.

`oox/source/core/contexthandler2.cxx` lists the namespaces LibreOffice claims for an
mc:Choice. When a Choice it accepts holds a p:oleObj whose relationship is external, it
gets a linked object it cannot reach and draws nothing; we do not claim VML, take the
mc:Fallback, and draw the replacement picture in it. Ours is the better page and the
difference is all ink.

Prints one row per slide carrying one: document, slide number, the Requires value.
"""
import os, re, sys, zipfile

CHOICE = re.compile(rb'<mc:Choice[^>]*\bRequires="([^"]+)"', re.S)
ALT = re.compile(rb'<mc:AlternateContent\b.*?</mc:AlternateContent>', re.S)

for root, _, files in os.walk(sys.argv[1]):
    for name in sorted(files):
        if not name.lower().endswith(('.pptx', '.pptm', '.ppsx', '.potx')):
            continue
        path = os.path.join(root, name)
        try:
            z = zipfile.ZipFile(path)
        except Exception:
            continue
        with z:
            for entry in z.namelist():
                m = re.fullmatch(r'ppt/slides/slide(\d+)\.xml', entry)
                if not m:
                    continue
                data = z.read(entry)
                for block in ALT.findall(data):
                    if b'oleObj' not in block:
                        continue
                    requires = CHOICE.search(block)
                    if not requires:
                        continue
                    print(f"{os.path.relpath(path, sys.argv[1])}\t{int(m.group(1))}"
                          f"\t{requires.group(1).decode()}")
                    break
