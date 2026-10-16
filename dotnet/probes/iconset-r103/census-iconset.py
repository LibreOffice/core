#!/usr/bin/env python3
"""Census every iconSet rule in the corpus, in BOTH spellings, and union the documents.

An `iconSet` rule can be stated twice over:

* as a main-namespace `<cfRule type="iconSet">` inside `<conditionalFormatting sqref=...>`;
* as an `<x14:cfRule type="iconSet">` inside the worksheet's `extLst`, carrying its own
  `<xm:sqref>` and needing no main-namespace partner at all -- 26.2.4.2 imports and paints it
  regardless (`sc/source/filter/oox/extlstcontext.cxx`:165-194, this tree: "an ext entry does
  not need to have an existing corresponding entry").

An x14 rule whose `id` is claimed by some main-namespace rule's `<x14:id>` is an *extension* of
that rule and is not a rule of its own.  The two populations are disjoint on this corpus, so a
census that reads only one spelling undercounts the documents; report the union.

Usage: census-iconset.py [root ...]
"""
import collections
import os
import sys
import xml.etree.ElementTree as ET
import zipfile

NS = '{http://schemas.openxmlformats.org/spreadsheetml/2006/main}'
X14 = '{http://schemas.microsoft.com/office/spreadsheetml/2009/9/main}'
XM = '{http://schemas.microsoft.com/office/excel/2006/main}'

roots = sys.argv[1:] or ['/home/user/sample-files']
files = []
for root in roots:
    for dirpath, _dirs, names in os.walk(root):
        files += [os.path.join(dirpath, n) for n in names]
files = sorted(set(files))

rows = []
main_docs, ext_docs = set(), set()
n_main = n_ext = n_attached = 0
sets_used = collections.Counter()
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
    for n in sorted(sheets):
        try:
            root = ET.fromstring(z.read(n))
        except Exception:
            continue

        claimed = set()
        for blk in root.iter(NS + 'conditionalFormatting'):
            sq = blk.get('sqref') or ''
            for r in blk.iter(NS + 'cfRule'):
                for i in r.iter(X14 + 'id'):
                    claimed.add((i.text or '').strip())
                if (r.get('type') or '') != 'iconSet':
                    continue
                n_main += 1
                main_docs.add(f)
                body = r.find(NS + 'iconSet')
                a = dict(body.attrib) if body is not None else {}
                cfvo = ([(v.get('type'), v.get('val')) for v in body.findall(NS + 'cfvo')]
                        if body is not None else [])
                icons = ([(c.get('iconSet'), c.get('iconId')) for c in body.findall(NS + 'cfIcon')]
                         if body is not None else [])
                name = a.get('iconSet', '3TrafficLights1')
                sets_used[name] += 1
                rows.append((os.path.basename(f), n, 'main', sq, r.get('priority'),
                             name, a, cfvo, icons))

        # `xm:sqref` is a child of the enclosing `x14:conditionalFormatting`, not of the rule.
        for blk in root.iter(X14 + 'conditionalFormatting'):
          sq = (blk.findtext(XM + 'sqref') or '').strip()
          for r in blk.iter(X14 + 'cfRule'):
            if (r.get('type') or '') != 'iconSet':
                continue
            if (r.get('id') or '').strip() in claimed:
                n_attached += 1
                continue
            n_ext += 1
            ext_docs.add(f)
            body = r.find(X14 + 'iconSet')
            a = dict(body.attrib) if body is not None else {}
            cfvo = ([(v.get('type'), (v.findtext(XM + 'f') or v.get('val') or '').strip())
                     for v in body.findall(X14 + 'cfvo')] if body is not None else [])
            icons = ([(c.get('iconSet'), c.get('iconId')) for c in body.findall(X14 + 'cfIcon')]
                     if body is not None else [])
            name = a.get('iconSet', '3TrafficLights1')
            sets_used[name] += 1
            rows.append((os.path.basename(f), n, 'x14-only', sq, r.get('priority'),
                         name, a, cfvo, icons))

print('documents opened as OPC spreadsheets:', zips)
print(f'iconSet: main-namespace rules {n_main} in {len(main_docs)} documents; '
      f'x14 extensions of those {n_attached}; '
      f'x14-only rules {n_ext} in {len(ext_docs)} documents')
print(f'union of documents: {len(main_docs | ext_docs)}   '
      f'intersection: {len(main_docs & ext_docs)}   total rules: {n_main + n_ext}')
print()
print('icon sets named, by rule:')
for k, v in sorted(sets_used.items(), key=lambda kv: (-kv[1], kv[0])):
    print(f'  {k:20} {v}')
print()
for r in sorted(rows):
    print(f'{r[0]} | {r[1]} | {r[2]} | sqref {r[3]!r} | priority {r[4]} | set {r[5]}')
    print(f'    attrs {r[6]}')
    print(f'    cfvo  {r[7]}')
    print(f'    icons {r[8]}')
print()
print('documents (union):')
for f in sorted(main_docs | ext_docs):
    tag = []
    if f in main_docs:
        tag.append('main')
    if f in ext_docs:
        tag.append('x14')
    print(f'  {"+".join(tag):9} {f}')
