#!/usr/bin/env python3
import sys
import xml.etree.ElementTree as ET
NS={'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'table':'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
 'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}
def q(t):
    p,l=t.split(':'); return '{%s}%s'%(NS[p],l)
t=ET.parse(sys.argv[1]); r=t.getroot()
sty={}
for st in r.iter(q('style:style')):
    if st.get(q('style:family'))!='table-cell': continue
    n=st.get(q('style:name'))
    tp=st.find(q('style:text-properties'))
    d={'parent':st.get(q('style:parent-style-name')),
       'maps':[m.get(q('style:base-cell-address')) for m in st.findall(q('style:map'))],
       'mapapply':[m.get(q('style:apply-style-name')) for m in st.findall(q('style:map'))],
       'cond':[m.get(q('style:condition')) for m in st.findall(q('style:map'))]}
    if tp is not None:
        for a in ('fo:font-size','style:font-size-asian','style:font-size-complex',
                  'style:font-name','style:font-name-asian','style:font-name-complex','fo:font-family'):
            v=tp.get(q(a))
            if v: d[a]=v
    cp=st.find(q('style:table-cell-properties'))
    if cp is not None:
        for a in ('fo:wrap-option','style:vertical-align','style:rotation-angle'):
            v=cp.get(q(a))
            if v: d[a]=v
    sty[n]=d
def resolve(n,key,seen=None):
    seen=seen or set()
    while n and n not in seen:
        seen.add(n)
        d=sty.get(n)
        if not d: return None
        if key in d: return (n,d[key])
        n=d['parent']
    return None
names=sys.argv[2:] if len(sys.argv)>2 else sorted(sty)
for n in names:
    d=sty.get(n)
    if d is None: print(n,'MISSING'); continue
    print('%-22s parent=%-22s size=%-22s wrap=%-6s maps=%s %s'%(
        n,d.get('parent'),str(resolve(n,'fo:font-size')),d.get('fo:wrap-option'),d['mapapply'],d['cond']))
