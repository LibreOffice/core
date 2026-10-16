import re,zlib,sys,os,collections
def streams(path):
    data=open(path,'rb').read(); outs=[]
    for m in re.finditer(rb'stream\r?\n',data):
        s=m.end(); e=data.find(b'endstream',s)
        try: d=zlib.decompress(data[s:e])
        except Exception: continue
        if b'BT' in d and b'Tf' in d: outs.append(d.decode('latin-1'))
    return outs
def runs(path):
    """(page,x,y) -> rgb for every text-showing BT block"""
    out={}
    for i,t in enumerate(streams(path),1):
        col=(0.0,0.0,0.0)
        for m in re.finditer(r'([\d.]+ [\d.]+ [\d.]+) rg|BT\s+([\d.-]+) ([\d.-]+) Td', t):
            if m.group(1): col=tuple(round(float(v),2) for v in m.group(1).split())
            else: out[(i, round(float(m.group(2))), round(float(m.group(3))))]=col
    return out
def near(ref,key,tol=12):
    p,x,y=key
    best=None;bd=None
    for (pp,xx,yy),c in ref.items():
        if pp!=p: continue
        d=abs(xx-x)+abs(yy-y)
        if d<=tol and (bd is None or d<bd): bd,best=d,c
    return best
REFDIR='/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides'
tb=tw=tl=tu=0; docs=0
for name in [l.rstrip("\n") for l in open("/tmp/arch-check/changed.txt")]:
    b=runs(f'/tmp/arch-check/reach/before/{name}')
    a=runs(f'/tmp/arch-check/reach/after/{name}')
    rp=os.path.join(REFDIR,name)
    if not os.path.exists(rp): print('no ref',name); continue
    r=runs(rp)
    moved=[k for k in b if k in a and b[k]!=a[k]]
    better=worse=lateral=unknown=0
    for k in moved:
        rc=near(r,k)
        if rc is None: unknown+=1
        elif rc==a[k] and rc!=b[k]: better+=1
        elif rc==b[k] and rc!=a[k]: worse+=1
        else: lateral+=1
    docs+=1; tb+=better; tw+=worse; tl+=lateral; tu+=unknown
    print(f"{name[:62]:<64} moved {len(moved):>4}  ->ref {better:>4}  away {worse:>3}  neither {lateral:>3}  unmatched {unknown:>3}")
print(f"\nTOTAL over {docs} documents: moved-to-reference {tb}, moved-away {tw}, neither {tl}, unmatched {tu}")
