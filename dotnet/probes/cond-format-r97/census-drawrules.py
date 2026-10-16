#!/usr/bin/env python3
"""Census every dataBar and iconSet rule in the corpus, by content and including the x14 list.

Two corrections to r96's census, both of which change the reach figure:

* it filtered on `.xlsx`/`.xlsm`, so it could not see an OPC zip wearing a `.xls` name;
* it read only the main-namespace `cfRule`, and an `x14:cfRule` in a worksheet's `extLst` can
  be a **rule of its own** rather than an extension of one. An `x14` rule whose `id` matches
  some main-namespace rule's `<x14:id>` is an extension of that rule and is not counted again;
  one with no such partner is a rule the reference imports and nothing else states.

Usage: census-drawrules.py [root ...]
"""
import collections
import os
import sys
import xml.etree.ElementTree as ET
import zipfile

NS = '{http://schemas.openxmlformats.org/spreadsheetml/2006/main}'
X14 = '{http://schemas.microsoft.com/office/spreadsheetml/2009/9/main}'
XM = '{http://schemas.microsoft.com/office/excel/2006/main}'
KINDS = ('dataBar', 'iconSet')

roots = sys.argv[1:] or ['/home/user/sample-files']
files = []
for root in roots:
    for dirpath, _dirs, names in os.walk(root):
        files += [os.path.join(dirpath, n) for n in names]
files = sorted(set(files))

main_rules = collections.Counter()
main_docs = collections.defaultdict(set)
ext_only = collections.Counter()
ext_docs = collections.defaultdict(set)
ext_attached = collections.Counter()
detail = []
zips = 0

for f in files:
    try:
        z = zipfile.ZipFile(f)
    except Exception:
        continue
    sheets = [n for n in z.namelist()
              if n.startswith('xl/worksheets/') and n.endswith('.xml')]
    if not sheets:
        continue
    zips += 1
    for n in sheets:
        try:
            root = ET.fromstring(z.read(n))
        except Exception:
            continue

        # main-namespace rules, and the x14 ids they claim as their own extension
        claimed = set()
        for blk in root.iter(NS + 'conditionalFormatting'):
            sq = blk.get('sqref') or ''
            for r in blk.iter(NS + 'cfRule'):
                t = r.get('type') or '?'
                for i in r.iter(X14 + 'id'):
                    claimed.add((i.text or '').strip())
                if t not in KINDS:
                    continue
                main_rules[t] += 1
                main_docs[t].add(f)
                body = r.find(NS + t)
                cfvo, colours, attrs = [], [], {}
                if body is not None:
                    attrs = dict(body.attrib)
                    cfvo = [(v.get('type'), v.get('val')) for v in body.findall(NS + 'cfvo')]
                    colours = [c.attrib for c in body.findall(NS + 'color')]
                detail.append((os.path.basename(f), n, 'main', t, sq,
                               r.get('priority'), attrs, cfvo, colours))

        for r in root.iter(X14 + 'cfRule'):
            t = r.get('type') or '?'
            if t not in KINDS:
                continue
            rid = (r.get('id') or '').strip()
            if rid in claimed:
                ext_attached[t] += 1
                continue
            ext_only[t] += 1
            ext_docs[t].add(f)
            body = r.find(X14 + t)
            sq = ''
            parent = r
            attrs, cfvo, icons = {}, [], []
            if body is not None:
                attrs = dict(body.attrib)
                cfvo = [(v.get('type'), (v.findtext(XM + 'f') or '').strip())
                        for v in body.findall(X14 + 'cfvo')]
                icons = [(c.get('iconSet'), c.get('iconId'))
                         for c in body.findall(X14 + 'cfIcon')]
            detail.append((os.path.basename(f), n, 'x14-only', t, sq,
                           r.get('priority'), attrs, cfvo, icons))

print('documents opened as OPC spreadsheets:', zips)
for t in KINDS:
    print(f'{t:8} main-namespace rules {main_rules[t]:3d} in {len(main_docs[t]):2d} documents; '
          f'x14 extensions of those {ext_attached[t]:3d}; '
          f'x14-only rules {ext_only[t]:3d} in {len(ext_docs[t]):2d} documents')
print()
for d in sorted(detail):
    print(f'{d[0]} | {d[1]} | {d[2]} {d[3]} | sqref {d[4]!r} | priority {d[5]}')
    print(f'    attrs {d[6]}')
    print(f'    cfvo  {d[7]}')
    print(f'    extra {d[8]}')
