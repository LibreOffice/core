#!/usr/bin/env python3
"""Which corpus documents carry a legacy FORMCHECKBOX form field."""
import os,sys,zipfile,collections,re
root=sys.argv[1]
tot=collections.Counter(); hits=[]
for dirpath,_,files in os.walk(root):
    for f in sorted(files):
        p=os.path.join(dirpath,f); low=f.lower()
        n=0
        if low.endswith(('.docx','.docm')):
            tot['docx']+=1
            try: z=zipfile.ZipFile(p)
            except Exception: continue
            for it in z.namelist():
                if it.startswith('word/') and it.endswith('.xml'):
                    n+=z.read(it).count(b'FORMCHECKBOX')
        elif low.endswith('.doc'):
            tot['doc']+=1
            b=open(p,'rb').read()
            n=b.count('FORMCHECKBOX'.encode('utf-16-le'))+b.count(b'FORMCHECKBOX')
        else:
            continue
        if n: hits.append((n,os.path.relpath(p,root)))
hits.sort(reverse=True)
for n,d in hits: print(n,d)
print('#',len(hits),'of',dict(tot))
