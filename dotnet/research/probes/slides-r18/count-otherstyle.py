#!/usr/bin/env python3
"""How many corpus decks would the p:otherStyle rung change, and by how much.

Counts, per PPTX-family deck in the slides track: whether its master states a populated
p:otherStyle, whether the presentation states a p:defaultTextStyle, and whether the two
disagree about lvl1's sz.
"""
import os
import re
import sys
import zipfile

root = sys.argv[1] if len(sys.argv) > 1 else '/workspace/sample-files/slides'

SZ = re.compile(r'<a:lvl1pPr\b[^>]*>(.*?)</a:lvl1pPr>', re.S)
SZV = re.compile(r'sz="(\d+)"')


def lvl1_sz(xml, container):
    m = re.search(r'<%s>(.*?)</%s>' % (container, container), xml, re.S)
    if not m:
        return None, False
    body = m.group(1)
    if not body.strip():
        return None, True
    lm = SZ.search(body)
    if not lm:
        return None, True
    sm = SZV.search(lm.group(1))
    return (int(sm.group(1)) if sm else None), True


total = with_other = with_default = disagree = 0
rows = []
for dirpath, _dirs, files in os.walk(root):
    for name in sorted(files):
        if os.path.splitext(name)[1].lower() not in ('.pptx', '.pptm', '.ppsx', '.potx', '.ppsm'):
            continue
        path = os.path.join(dirpath, name)
        total += 1
        try:
            with zipfile.ZipFile(path) as z:
                names = z.namelist()
                masters = [n for n in names if re.fullmatch(r'ppt/slideMasters/slideMaster\d+\.xml', n)]
                other = None
                has_other = False
                for m in masters:
                    sz, present = lvl1_sz(z.read(m).decode('utf-8', 'replace'), 'p:otherStyle')
                    if present:
                        has_other = True
                        if sz is not None:
                            other = sz
                            break
                pres = 'ppt/presentation.xml'
                dsz, dpresent = (None, False)
                if pres in names:
                    dsz, dpresent = lvl1_sz(z.read(pres).decode('utf-8', 'replace'), 'p:defaultTextStyle')
        except Exception as exc:                                   # noqa: BLE001 — a probe
            print(f'unreadable {name}: {exc}')
            continue
        if has_other:
            with_other += 1
        if dpresent:
            with_default += 1
        if other is not None and dsz is not None and other != dsz:
            disagree += 1
            rows.append((name, other, dsz))

print(f'pptx-family decks                       {total}')
print(f'  master states a p:otherStyle          {with_other}')
print(f'  presentation states defaultTextStyle  {with_default}')
print(f'  the two disagree about lvl1 sz        {disagree}')
for name, o, d in sorted(rows, key=lambda r: -abs(r[1] - r[2])):
    print(f'    other {o:5d}  default {d:5d}   {name}')
