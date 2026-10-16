import os,zipfile
from xml.etree import ElementTree as ET
D='{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}'
T='{urn:oasis:names:tc:opendocument:xmlns:table:1.0}'
S='{urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0}'
ROOT='/home/user/corpus-odf/sheets'
def shapes(c,out):
    for ch in c:
        if not ch.tag.startswith(D): continue
        l=ch.tag[len(D):]
        if l in ('g','a'): shapes(ch,out)
        else: out.append((l,ch))
seen=set(); files=[]
for dp,_,ns in os.walk(ROOT):
    for f in ns:
        if f.lower().endswith('.ods'):
            p=os.path.join(dp,f); st=os.stat(p)
            if (st.st_dev,st.st_ino) in seen: continue
            seen.add((st.st_dev,st.st_ino)); files.append(p)
tot=0; docs={}
for p in sorted(files):
    try:
        with zipfile.ZipFile(p) as z: t=ET.fromstring(z.read('content.xml'))
    except Exception: continue
    n=0
    for cell in t.iter():
        if cell.tag not in (T+'table-cell',T+'covered-table-cell'): continue
        o=[];shapes(cell,o)
        for k,el in o:
            if el.get(D+'transform') and el.get(S+'x') is None and el.get(T+'end-cell-address'):
                n+=1
    if n: docs[p]=n; tot+=n
print('shapes stating a transform, no svg:x and an end cell: %d in %d documents'%(tot,len(docs)))
for p,n in sorted(docs.items(), key=lambda kv:-kv[1]):
    print('%5d  %s'%(n, os.path.relpath(p,ROOT)))
