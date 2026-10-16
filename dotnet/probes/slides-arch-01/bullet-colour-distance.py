import re,zlib,os,collections
def streams(path):
    data=open(path,'rb').read(); outs=[]
    for m in re.finditer(rb'stream\r?\n',data):
        s=m.end(); e=data.find(b'endstream',s)
        try: d=zlib.decompress(data[s:e])
        except Exception: continue
        if b'BT' in d and b'Tf' in d: outs.append(d.decode('latin-1'))
    return outs
def colours(path):
    c=collections.Counter()
    for t in streams(path):
        col=(0.0,0.0,0.0)
        for m in re.finditer(r'([\d.]+ [\d.]+ [\d.]+) rg|BT\s+([\d.-]+) ([\d.-]+) Td', t):
            if m.group(1): col=tuple(round(float(v),2) for v in m.group(1).split())
            else: c[col]+=1
    return c
def dist(x,y):  # symmetric multiset difference
    return sum(((x-y)+(y-x)).values())
REF='/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides'
better=worse=equal=0
for name in [l.strip() for l in open('/tmp/arch-check/changed.txt')]:
    r=colours(os.path.join(REF,name))
    b=colours(f'/tmp/arch-check/reach/before/{name}')
    a=colours(f'/tmp/arch-check/reach/after/{name}')
    db,da=dist(b,r),dist(a,r)
    tag='better' if da<db else ('WORSE' if da>db else 'equal')
    if da<db: better+=1
    elif da>db: worse+=1
    else: equal+=1
    print(f"{name[:60]:<62} colour-run distance to reference  before {db:>5}  after {da:>5}   {tag}")
print(f"\n{better} documents closer to the reference, {worse} further, {equal} unchanged in distance")
