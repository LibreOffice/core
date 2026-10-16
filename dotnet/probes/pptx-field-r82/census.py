#!/usr/bin/env python3
"""Census a:hlinkClick across the original corpus's OOXML documents.

Counts occurrences, and classifies each by whether LibreOffice's HyperLinkContext
(oox/source/drawingml/hyperlinkcontext.cxx:40-156) would leave the run's
maHyperlinkPropertyMap non-empty -- which is the only thing textrun.cxx:88 tests
before building a com.sun.star.text.TextField.URL.
"""
import sys, zipfile, re, collections
from pathlib import Path
import xml.etree.ElementTree as ET

A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
R = '{http://schemas.openxmlformats.org/officeDocument/2006/relationships}'
REL = '{http://schemas.openxmlformats.org/package/2006/relationships}'

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')

def rels_for(zf, part):
    p = Path(part)
    rp = str(p.parent / '_rels' / (p.name + '.rels'))
    try:
        data = zf.read(rp)
    except KeyError:
        return {}
    out = {}
    for r in ET.fromstring(data):
        out[r.get('Id')] = (r.get('Target') or '', r.get('TargetMode') or 'Internal')
    return out

def makes_field(el, rels):
    """Would HyperLinkContext set at least one property?"""
    rid = el.get(R + 'id')
    if rid:
        tgt, mode = rels.get(rid, ('', ''))
        if tgt:
            return True          # PROP_URL, external or internal
    if el.get('tooltip'):   return True
    if el.get('tgtFrame'):  return True
    if el.get('action'):    return True   # PROP_Action, always set when non-empty
    if el.get('invalidUrl'):return True
    if el.get('history') in ('0', 'false'):       return True
    if el.get('highlightClick') in ('1','true'):  return True
    if el.get('endSnd') in ('1','true'):          return True
    if el.find(A + 'extLst') is not None:         return True   # sets PROP_CharColor
    return False

rows = []
for path in sorted(ROOT.rglob('*')):
    if not path.is_file(): continue
    ext = path.suffix.lower().lstrip('.')
    if ext not in ('pptx','pptm','potx','ppsx','docx','docm','xlsx','xlsm','xltx'):
        continue
    try:
        zf = zipfile.ZipFile(path)
    except Exception:
        continue
    total = field = empty = 0
    withtext = 0
    longest = 0
    parts = collections.Counter()
    for name in zf.namelist():
        if not name.endswith('.xml'): continue
        try: data = zf.read(name)
        except Exception: continue
        if b'hlinkClick' not in data: continue
        try: root = ET.fromstring(data)
        except Exception: continue
        rels = rels_for(zf, name)
        # every a:rPr carrying an a:hlinkClick; the run's text is its sibling a:t
        for parent in root.iter():
            for child in list(parent):
                if child.tag != A + 'hlinkClick': continue
                total += 1
                parts[name.split('/')[1] if '/' in name else name] += 1
                if makes_field(child, rels):
                    field += 1
                    # parent is a:rPr; its parent is a:r with an a:t
                    pass
                else:
                    empty += 1
    if total:
        rows.append((str(path.relative_to(ROOT)), ext, total, field, empty))
    zf.close()

print('path\text\thlinkClick\tfield\tinert')
for r in rows:
    print('\t'.join(str(x) for x in r))

by = collections.defaultdict(lambda: [0,0,0,0])
for _, ext, t, f, e in rows:
    b = by[ext]; b[0]+=1; b[1]+=t; b[2]+=f; b[3]+=e
print('\n# ext\tdocs\thlinkClick\tfield\tinert', file=sys.stderr)
for ext, b in sorted(by.items()):
    print(f'# {ext}\t{b[0]}\t{b[1]}\t{b[2]}\t{b[3]}', file=sys.stderr)
