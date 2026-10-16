#!/usr/bin/env python3
"""Reach of a:hlinkClick, by part kind, with the run text each one carries.

A hyperlink run becomes a com.sun.star.text.TextField.URL only when
HyperLinkContext left at least one property on the run's maHyperlinkPropertyMap
(oox/source/drawingml/hyperlinkcontext.cxx:40-156, tested at
oox/source/drawingml/textrun.cxx:88).  The field's Representation is the run's
own a:t text (textrun.cxx:157), so one a:r is one field however the link is
split across runs.
"""
import sys, zipfile, collections
from pathlib import Path
import xml.etree.ElementTree as ET

A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
R = '{http://schemas.openxmlformats.org/officeDocument/2006/relationships}'
ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')

def rels_for(zf, part):
    p = Path(part)
    rp = str(p.parent / '_rels' / (p.name + '.rels'))
    try: data = zf.read(rp)
    except KeyError: return {}
    return {r.get('Id'): (r.get('Target') or '') for r in ET.fromstring(data)}

def makes_field(el, rels):
    rid = el.get(R + 'id')
    if rid and rels.get(rid): return True
    for a in ('tooltip','tgtFrame','action','invalidUrl'):
        if el.get(a): return True
    if el.get('history') in ('0','false'): return True
    if el.get('highlightClick') in ('1','true'): return True
    if el.get('endSnd') in ('1','true'): return True
    if el.find(A + 'extLst') is not None: return True
    return False

def kind(name):
    if name.startswith('ppt/slides/slide'): return 'slide'
    if name.startswith('ppt/slideLayouts'): return 'slideLayout'
    if name.startswith('ppt/slideMasters'): return 'slideMaster'
    if name.startswith('ppt/notesSlides'): return 'notesSlide'
    if name.startswith('ppt/charts') or '/charts/' in name: return 'chart'
    if name.startswith('ppt/diagrams') or '/diagrams/' in name: return 'diagram'
    if name.startswith('word/'): return 'word:' + name.split('/')[-1]
    if name.startswith('xl/'): return 'xl:' + ('/'.join(name.split('/')[1:2]))
    return name

perdoc = collections.defaultdict(lambda: collections.Counter())
lengths = collections.Counter()
docs_by_kind = collections.defaultdict(set)
inert_rows = []
runtexts = []

for path in sorted(ROOT.rglob('*')):
    if not path.is_file(): continue
    ext = path.suffix.lower().lstrip('.')
    if ext not in ('pptx','pptm','potx','ppsx','docx','docm','xlsx','xlsm','xltx'): continue
    try: zf = zipfile.ZipFile(path)
    except Exception: continue
    rel = str(path.relative_to(ROOT))
    for name in zf.namelist():
        if not name.endswith('.xml'): continue
        try: data = zf.read(name)
        except Exception: continue
        if b'hlinkClick' not in data: continue
        try: root = ET.fromstring(data)
        except Exception: continue
        rels = rels_for(zf, name)
        k = kind(name)
        for r in root.iter(A + 'r'):
            rpr = r.find(A + 'rPr')
            if rpr is None: continue
            hl = rpr.find(A + 'hlinkClick')
            if hl is None: continue
            t = r.find(A + 't')
            text = t.text or '' if t is not None else ''
            if makes_field(hl, rels):
                perdoc[rel][k] += 1
                docs_by_kind[k].add(rel)
                lengths[len(text)] += 1
                if k == 'slide':
                    runtexts.append((rel, len(text), text[:120]))
            else:
                perdoc[rel][k + '/inert'] += 1
                inert_rows.append((rel, name, ET.tostring(hl).decode()[:200]))
    zf.close()

print('# field-making a:hlinkClick runs, by part kind', file=sys.stderr)
for k, docs in sorted(docs_by_kind.items()):
    n = sum(c[k] for c in perdoc.values())
    print(f'# {k}\t{n} runs\t{len(docs)} documents', file=sys.stderr)
print(f'# inert (map stays empty): {len(inert_rows)}', file=sys.stderr)
for r in inert_rows: print('# INERT ' + '\t'.join(r), file=sys.stderr)

# distribution of run-text lengths on slides
print('\n# slide-run text length distribution', file=sys.stderr)
buckets = collections.Counter()
for _, n, _ in runtexts:
    b = 0 if n == 0 else (1 if n < 20 else (2 if n < 40 else (3 if n < 80 else 4)))
    buckets[b] += 1
names = {0:'empty',1:'1-19',2:'20-39',3:'40-79',4:'80+'}
for b in sorted(buckets): print(f'# {names[b]}\t{buckets[b]}', file=sys.stderr)

print('doc\tlen\ttext')
for rel, n, t in sorted(runtexts, key=lambda x: -x[1])[:60]:
    print(f'{rel}\t{n}\t{t}')
