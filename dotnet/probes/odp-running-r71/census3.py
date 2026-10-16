#!/usr/bin/env python3
"""Census 3: what a master's presentation:class frames actually hold.

(a) the four running kinds -- which field elements they carry;
(b) every other class -- whether it is an empty placeholder (which LibreOffice suppresses
    when printing, sdpage.cxx:2941-2947) or carries text (which it would draw).

Environment: corpus /home/user/corpus-odf, XML only.
"""
import sys, zipfile, collections
from pathlib import Path
from xml.etree import ElementTree as ET

NS = {
 'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'draw':'urn:oasis:names:tc:opendocument:xmlns:drawing:1.0',
 'pres':'urn:oasis:names:tc:opendocument:xmlns:presentation:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
 'drawooo':'http://openoffice.org/2010/draw',
}
def q(p):
    a,b=p.split(':'); return '{%s}%s'%(NS[a],b)
RUNNING={'footer','date-time','page-number','header'}

corpus=Path(sys.argv[1])
fields=collections.Counter(); other=collections.Counter()
otherdocs=collections.defaultdict(set); srcs=collections.Counter()
nonempty=[]
for doc in sorted(corpus.rglob('*.odp')):
    rel=str(doc.relative_to(corpus))
    try:
        z=zipfile.ZipFile(doc); styles=ET.fromstring(z.read('styles.xml'))
        content=ET.fromstring(z.read('content.xml'))
    except Exception: continue
    for d in content.iter():
        if d.tag in (q('pres:date-time-decl'),):
            srcs[d.get(q('pres:source'))]+=1
    for mp in styles.iter(q('style:master-page')):
        for el in mp:
            if el.tag==q('pres:notes'): continue
            cls=el.get(q('pres:class'))
            if cls is None: continue
            if cls in RUNNING:
                kinds=set()
                for ch in el.iter():
                    t=ch.tag
                    if t==q('text:page-number'): kinds.add('text:page-number')
                    elif t==q('pres:footer'): kinds.add('presentation:footer')
                    elif t==q('pres:header'): kinds.add('presentation:header')
                    elif t==q('pres:date-time'): kinds.add('presentation:date-time')
                    elif t==q('text:date'): kinds.add('text:date')
                    elif t==q('text:time'): kinds.add('text:time')
                txt=''.join(el.itertext()).strip()
                key=(cls, ','.join(sorted(kinds)) or '-', 'text' if txt else 'empty')
                fields[key]+=1
            else:
                txt=''.join(el.itertext()).strip()
                ph=el.get(q('pres:placeholder'))
                key=(cls, 'placeholder='+str(ph), 'text' if txt else 'empty')
                other[key]+=1
                otherdocs[key].add(rel)
                if txt and ph!='true':
                    nonempty.append((rel,cls,txt[:60]))
print('== running kinds: (class, fields, has-text) ==')
for k,v in sorted(fields.items()): print('%-14s %-40s %-6s %d'%(k[0],k[1],k[2],v))
print()
print('== other classes ==')
for k,v in sorted(other.items()): print('%-14s %-22s %-6s %5d  docs %d'%(k[0],k[1],k[2],v,len(otherdocs[k])))
print()
print('== date-time-decl source values ==', dict(srcs))
print()
print('== non-placeholder master presentation frames carrying text: %d ==' % len(nonempty))
for r in nonempty[:40]: print('  %-70s %-12s %s' % (r[0].split('/')[-1][:70], r[1], r[2]))
