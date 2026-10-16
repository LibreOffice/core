#!/usr/bin/env python3
"""Which style:graphic-properties the 137 inked sheet shapes of the converted `.ods` column
state, and with what values. The four insets, the two text-area adjustments and the wrap are
on every one of them; style:overflow-behavior is on 95.
"""
import os,zipfile,collections
from xml.etree import ElementTree as ET
D='{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}'
T='{urn:oasis:names:tc:opendocument:xmlns:table:1.0}'
X='{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'
S='{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
ROOT='/home/user/corpus-odf/sheets'
def shapes(c,out):
    for ch in c:
        if not ch.tag.startswith(D): continue
        l=ch.tag[len(D):]
        if l in ('g','a'): shapes(ch,out)
        else: out.append((l,ch))
attrs=collections.Counter(); vals=collections.defaultdict(collections.Counter)
seen=set(); files=[]
for dp,_,ns in os.walk(ROOT):
    for f in ns:
        if f.lower().endswith('.ods'):
            p=os.path.join(dp,f); st=os.stat(p)
            if (st.st_dev,st.st_ino) in seen: continue
            seen.add((st.st_dev,st.st_ino)); files.append(p)
for p in sorted(files):
    try:
        with zipfile.ZipFile(p) as z: t=ET.fromstring(z.read('content.xml'))
    except Exception: continue
    styles={}
    for st in t.iter(S+'style'):
        styles[(st.get(S+'name'), st.get(S+'family'))]=st
    for cell in t.iter():
        if cell.tag not in (T+'table-cell',T+'covered-table-cell'): continue
        o=[];shapes(cell,o)
        for k,el in o:
            txt=''.join(''.join(pp.itertext()) for pp in el.findall('.//'+X+'p'))
            if not txt.strip(): continue
            gs=styles.get((el.get(D+'style-name'),'graphic'))
            if gs is None: attrs['(no graphic style in content.xml)']+=1; continue
            gp=gs.find(S+'graphic-properties')
            if gp is None: attrs['(no graphic-properties)']+=1; continue
            for a,v in gp.attrib.items():
                nm=a.split('}')[-1]; pre=a.split('}')[0][1:]
                key=('fo:' if 'xsl-fo' in pre else 'draw:' if 'drawing' in pre else 'style:' if 'style:1.0' in pre else 'loext:' if 'loext' in pre else pre[-12:]+':')+nm
                attrs[key]+=1
                if nm in ('textarea-vertical-align','textarea-horizontal-align','wrap-option','overflow-behavior','auto-grow-height','auto-grow-width','fit-to-size','shrink-to-fit'):
                    vals[key][v]+=1
for k,v in attrs.most_common(40): print('%6d %s'%(v,k))
print()
for k in sorted(vals): print(k, dict(vals[k]))
