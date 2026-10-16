#!/usr/bin/env python3
"""Every show-text operator with its device pen, its /Fn resource name and its Tf size."""
import re, sys
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
from pg import page_stream
NUM = rb'-?\d*\.?\d+'
def mul(a,b):
    return [a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
            a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
            a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5]]
def runs(stream):
    ctm=[1.0,0,0,1.0,0,0]; stack=[]; tm=tlm=[1.0,0,0,1.0,0,0]
    fnt=None; size=0.0; out=[]; args=[]
    tok=re.compile(rb'(\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>|\[[^\]]*\]|'+NUM+rb'|/[^\s/\[\]<>()]+|[A-Za-z\'"*]+)')
    for m in tok.finditer(stream):
        t=m.group(1)
        if re.fullmatch(rb'[A-Za-z\'"*]+',t):
            op=t.decode('latin1')
            if op=='q': stack.append(list(ctm))
            elif op=='Q':
                if stack: ctm=stack.pop()
            elif op=='cm' and len(args)>=6: ctm=mul([float(x) for x in args[-6:]],ctm)
            elif op=='BT': tm=tlm=[1.0,0,0,1.0,0,0]
            elif op=='Tm' and len(args)>=6: tm=tlm=[float(x) for x in args[-6:]]
            elif op in ('Td','TD') and len(args)>=2:
                tlm=mul([1,0,0,1,float(args[-2]),float(args[-1])],tlm); tm=list(tlm)
            elif op=='Tf' and len(args)>=2:
                fnt=args[-2].decode('latin1'); size=float(args[-1])
            elif op in ('Tj','TJ',"'",'"'):
                d=mul(tm,ctm)
                out.append((d[4],d[5],fnt,size*abs(tm[0])*abs(ctm[0]),(args[-1] if args else b'').decode('latin1','replace')))
            args=[]
        else: args.append(t)
    return out
if __name__=='__main__':
    for x,y,f,s,txt in runs(page_stream(sys.argv[1],int(sys.argv[2])-1)):
        print(f"{x:9.2f} {y:9.2f}  {str(f):8s} {s:7.3f}  {txt[:60]}")
