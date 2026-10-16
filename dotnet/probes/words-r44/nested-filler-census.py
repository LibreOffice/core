#!/usr/bin/env python3
"""Count cells whose last child is an empty paragraph immediately following a nested table."""
import os,sys,zipfile,collections
import xml.etree.ElementTree as ET
W='{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
root=sys.argv[1]
tot=collections.Counter()
docs=[]
for dirpath,_,files in os.walk(root):
    for f in sorted(files):
        if not f.lower().endswith(('.docx','.docm')): continue
        p=os.path.join(dirpath,f)
        n=0
        try: z=zipfile.ZipFile(p)
        except Exception: continue
        for it in z.namelist():
            if not (it.startswith('word/') and it.endswith('.xml')): continue
            if not any(k in it for k in ('document','header','footer')): continue
            try: r=ET.fromstring(z.read(it))
            except Exception: continue
            for tc in r.iter(W+'tc'):
                kids=[c for c in tc if c.tag in (W+'tbl',W+'p')]
                if len(kids)>=2 and kids[-1].tag==W+'p' and kids[-2].tag==W+'tbl':
                    if not any(t.text for t in kids[-1].iter(W+'t')) and kids[-1].find(W+'r') is None:
                        n+=1
        if n: docs.append((n,os.path.relpath(p,root)))
        tot['docx']+=1
docs.sort(reverse=True)
for n,d in docs: print(n,d)
print(f'# {len(docs)} of {tot["docx"]} docx carry the shape')
