#!/usr/bin/env python3
import os, re, sys, zipfile
CORPUS="/home/user/corpus-odf"
FRAME=re.compile(r'<draw:frame\b([^>]*)>', re.S)
rows=[]
for root,_d,files in os.walk(CORPUS):
    for n in files:
        if not n.lower().endswith(".odt"): continue
        p=os.path.join(root,n)
        try:
            c=zipfile.ZipFile(p).read("content.xml").decode("utf-8","replace")
        except Exception: continue
        noh=brk=0
        for m in FRAME.finditer(c):
            a=m.group(1)
            if 'svg:width' in a and 'svg:height' not in a:
                noh+=1
                if 'may-break-between-pages="true"' in a: brk+=1
        if noh: rows.append((n,noh,brk))
print("documents with a height-less draw:frame:", len(rows))
print("total such frames:", sum(r[1] for r in rows), " of which may-break:", sum(r[2] for r in rows))
print("documents where every such frame may break:", sum(1 for r in rows if r[1]==r[2]))
print("documents where none may break:", sum(1 for r in rows if r[2]==0))
for r in sorted(rows,key=lambda r:-r[1])[:15]: print("  ",r)
