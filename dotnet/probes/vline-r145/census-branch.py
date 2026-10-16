#!/usr/bin/env python3
"""For every v:line inside an mc:Fallback, what does the mc:Choice beside it require and hold?

The question this answers is whether the v:line is reachable content at all. A reader that
resolves mc:AlternateContent to the choice never sees the fallback's VML; LibreOffice's
writerfilter and this tree both prefer a `Requires="wps"` choice.
"""
import sys, zipfile, pathlib, collections
from xml.etree import ElementTree as ET

VML = 'urn:schemas-microsoft-com:vml'
W   = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
MC  = 'http://schemas.openxmlformats.org/markup-compatibility/2006'

root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')
requires = collections.Counter()
choice_holds = collections.Counter()
no_choice = 0
total = 0
rows = []

for path in sorted(root.rglob('*')):
    if path.suffix.lower() not in ('.docx','.docm','.dotx','.dotm') or not path.is_file():
        continue
    try: z = zipfile.ZipFile(path)
    except Exception: continue
    for name in z.namelist():
        if not (name.startswith('word/') and name.endswith('.xml')): continue
        try: tree = ET.fromstring(z.read(name))
        except Exception: continue
        # map child -> parent
        parent = {c: p for p in tree.iter() for c in p}
        for ac in tree.iter(f'{{{MC}}}AlternateContent'):
            fb = ac.find(f'{{{MC}}}Fallback')
            if fb is None: continue
            nlines = sum(1 for _ in fb.iter(f'{{{VML}}}line'))
            if not nlines: continue
            total += nlines
            chs = ac.findall(f'{{{MC}}}Choice')
            if not chs:
                no_choice += nlines
                continue
            for ch in chs:
                req = ch.get('Requires','(none)')
                requires[req] += nlines
                kinds = set()
                for e in ch.iter():
                    ln = e.tag.split('}',1)[-1]
                    if ln in ('wsp','cNvCnPr','prstGeom','wgp','wpc'):
                        if ln == 'prstGeom': kinds.add('prstGeom:'+(e.get('prst') or '?'))
                        else: kinds.add(ln)
                choice_holds[tuple(sorted(kinds))] += nlines
                rows.append((path.name, name, req, nlines, ','.join(sorted(kinds))))
            break_ = None

print(f'v:line inside an mc:Fallback: {total}')
print(f'  ...with no mc:Choice beside it: {no_choice}')
print('Requires= on the choice (weighted by the fallback\'s v:line count):')
for k,v in requires.most_common(): print(f'   {k}\t{v}')
print('what the choice holds:')
for k,v in choice_holds.most_common(12): print(f'   {"|".join(k) or "(nothing recognised)"}\t{v}')
with open('vline-branch.tsv','w') as f:
    f.write('doc\tpart\trequires\tvlines_in_fallback\tchoice_holds\n')
    for r in rows: f.write('\t'.join(str(x) for x in r) + '\n')
