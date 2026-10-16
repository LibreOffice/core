#!/usr/bin/env python3
"""Census every v:line in the DOCX-family corpus, one row per occurrence.

Columns, per occurrence:
  doc            file name
  part           zip part the element lives in
  holder         w:pict | w:object | (none)
  depth          top | group (any v:group ancestor below the holder)
  alt            choice | fallback | plain   -- mc:AlternateContent branch, if any
  pos            the style's `position`, or '-'
  has_wh         style states BOTH width and height
  w,h            the stated width/height or '-'
  left,top       the stated left/top or '-'
  frm,to         the from/to attributes, '-' when absent
  parse          from/to both present and both parse as a pair of lengths
  strokeweight   attribute value or '-'
  strokecolor    attribute value or '-'
  stroked        the `stroked` attribute value or '-'
  strokeoff      1 when stroked="f"/"false" OR a child <v:stroke on="f">
"""
import sys, zipfile, pathlib, re
from xml.etree import ElementTree as ET

VML = 'urn:schemas-microsoft-com:vml'
W   = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
MC  = 'http://schemas.openxmlformats.org/markup-compatibility/2006'

LEN = re.compile(r'^\s*(-?[0-9]*\.?[0-9]+)\s*(pt|in|mm|cm|pc|pi|px|em|ex)?\s*$', re.I)

def style(el):
    out = {}
    for d in (el.get('style') or '').split(';'):
        if ':' in d:
            k, v = d.split(':', 1)
            out[k.strip().lower()] = v.strip()
    return out

def parses(v):
    if not v: return False
    parts = v.replace(' ', ',').split(',')
    parts = [p for p in parts if p != '']
    if len(parts) != 2: return False
    return all(LEN.match(p) for p in parts)

def run(root, out):
    rows = []
    for path in sorted(root.rglob('*')):
        if path.suffix.lower() not in ('.docx', '.docm', '.dotx', '.dotm') or not path.is_file():
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
            # walk the whole part, keeping each element's ancestor chain
            stack = [(tree, [])]
            while stack:
                el, chain = stack.pop()
                for child in el:
                    stack.append((child, chain + [el]))
                if el.tag != f'{{{VML}}}line':
                    continue
                tags = [a.tag for a in chain]
                holder = '-'
                for t in reversed(tags):
                    if t == f'{{{W}}}pict':   holder = 'w:pict';   break
                    if t == f'{{{W}}}object': holder = 'w:object'; break
                # depth: is there a v:group between the holder and here?
                depth = 'group' if f'{{{VML}}}group' in tags else 'top'
                alt = 'plain'
                if f'{{{MC}}}Fallback' in tags: alt = 'fallback'
                elif f'{{{MC}}}Choice' in tags: alt = 'choice'
                s = style(el)
                w, h = s.get('width', '-'), s.get('height', '-')
                lf, tp = s.get('left', '-'), s.get('top', '-')
                frm, to = el.get('from'), el.get('to')
                sw = el.get('strokeweight', '-')
                sc = el.get('strokecolor', '-')
                st = el.get('stroked', '-')
                off = 1 if str(st).lower() in ('f', 'false', '0') else 0
                for ch in el:
                    if ch.tag == f'{{{VML}}}stroke' and str(ch.get('on', '')).lower() in ('f','false','0'):
                        off = 1
                rows.append(dict(
                    doc=path.name, part=name, holder=holder, depth=depth, alt=alt,
                    pos=s.get('position', '-'),
                    has_wh=int(bool(s.get('width')) and bool(s.get('height'))),
                    w=w, h=h, left=lf, top=tp,
                    frm=frm or '-', to=to or '-',
                    parse=int(parses(frm) and parses(to)),
                    strokeweight=sw, strokecolor=sc, stroked=st, strokeoff=off,
                    fullpath=str(path)))
    cols = ['doc','part','holder','depth','alt','pos','has_wh','w','h','left','top',
            'frm','to','parse','strokeweight','strokecolor','stroked','strokeoff','fullpath']
    with open(out, 'w') as f:
        f.write('\t'.join(cols) + '\n')
        for r in rows:
            f.write('\t'.join(str(r[c]) for c in cols) + '\n')
    return rows

if __name__ == '__main__':
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')
    rows = run(root, sys.argv[2] if len(sys.argv) > 2 else 'vline-occurrences.tsv')
    print(f'{len(rows)} occurrences in {len(set(r["doc"] for r in rows))} documents')
