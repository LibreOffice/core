#!/usr/bin/env python3
"""How much rotated text a PDF holds, counted whichever way the producer expresses it.

LibreOffice turns text with the *text matrix* (`Tm`) and XRay.Office turns it with the *CTM*
(`cm`), so counting one of them alone reads the other producer as drawing nothing rotated.
That is exactly the trap this script exists to avoid: a first pass that counted `Tm` only
scored a correctly-rotated XRay.Office page as zero.
"""
import re, sys, zlib, math, collections

def streams(path):
    data=open(path,'rb').read(); out=[]
    for m in re.finditer(rb'stream\r?\n',data):
        s=m.end(); e=data.find(b'endstream',s)
        if e<0: continue
        try: out.append(zlib.decompress(data[s:e]))
        except Exception: pass
    return out

NUM=rb'(-?[\d.]+)'
pat=re.compile(rb' '.join([NUM]*6)+rb' (Tm|cm)')
for path in sys.argv[1:]:
    c=collections.Counter(); tot=0
    for st in streams(path):
        for m in pat.finditer(st):
            a,b=float(m.group(1)),float(m.group(2))
            if a==0 and b==0: continue
            ang=round(math.degrees(math.atan2(b,a)),1)
            if abs(ang)<0.05: continue
            c[(m.group(7).decode(),ang)]+=1; tot+=1
    print(f"{path.split('/')[-1]}: {tot} rotated matrices")
    for k,v in sorted(c.items()): print(f"    {k[0]} {k[1]:7.1f} deg : {v}")
