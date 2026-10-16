#!/usr/bin/env python3
"""Census the corpus's VML shape elements, for the two figures the fix rests on.

  1. How many `v:line` (and the other four the broken predicate admitted by
     accident) sit TOP-LEVEL in a `w:pict`/`w:object` -- i.e. would be dropped
     if `IsShape` were narrowed to the original five rather than widened to the
     ten real shape elements -- and how many of those state a sized `style`.
  2. How many are a direct child of a `v:group`, and whether `Group`'s own
     guard (left, top, width and height all present) would keep any of them.
"""
import re, sys, zipfile, pathlib
from xml.etree import ElementTree as ET

VML = 'urn:schemas-microsoft-com:vml'
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
WIDENED = {'line', 'polyline', 'curve', 'arc', 'image'}
ORIGINAL = {'shape', 'rect', 'roundrect', 'oval', 'group'}

def style(el):
    out = {}
    for d in (el.get('style') or '').split(';'):
        if ':' in d:
            k, v = d.split(':', 1)
            out[k.strip().lower()] = v.strip()
    return out

def sized(el):
    s = style(el)
    return bool(s.get('width')) and bool(s.get('height'))

root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')
top = {k: [0, 0] for k in WIDENED}      # [count, sized]
grp = {k: [0, 0] for k in WIDENED}      # [count, would pass Group's guard]
docs_top, docs_grp = set(), set()

for path in sorted(root.rglob('*')):
    if path.suffix.lower() not in ('.docx', '.docm', '.dotx') or not path.is_file():
        continue
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        continue
    for name in z.namelist():
        if not (name.startswith('word/') and name.endswith('.xml')):
            continue
        try:
            tree = ET.fromstring(z.read(name))
        except Exception:
            continue
        for holder in tree.iter():
            if holder.tag not in (f'{{{W}}}pict', f'{{{W}}}object'):
                continue
            # every descendant, with its chain back to the holder
            stack = [(holder, [])]
            while stack:
                el, chain = stack.pop()
                for child in el:
                    stack.append((child, chain + [el]))
                if not el.tag.startswith(f'{{{VML}}}'):
                    continue
                local = el.tag.split('}', 1)[1]
                if local not in WIDENED:
                    continue
                in_group = any(a.tag == f'{{{VML}}}group' for a in chain)
                if not in_group:
                    top[local][0] += 1
                    top[local][1] += sized(el)
                    docs_top.add(path.name)
                elif chain and chain[-1].tag == f'{{{VML}}}group':
                    s = style(el)
                    keeps = all(s.get(k) for k in ('left', 'top', 'width', 'height'))
                    grp[local][0] += 1
                    grp[local][1] += keeps
                    docs_grp.add(path.name)

print('element\ttop_level\ttop_level_sized\tgroup_member\tgroup_member_kept')
for k in sorted(WIDENED):
    print(f'{k}\t{top[k][0]}\t{top[k][1]}\t{grp[k][0]}\t{grp[k][1]}')
print(f'# documents with a top-level widened element: {len(docs_top)}')
print(f'# documents with one inside a v:group:        {len(docs_grp)}')
