import sys, zlib, re
def pages(data):
    # crude: find all stream objects, decompress, look for cm ... Do
    out=[]
    for m in re.finditer(rb'stream\r?\n', data):
        s=m.end()
        e=data.find(b'endstream', s)
        raw=data[s:e]
        try: c=zlib.decompress(raw)
        except Exception: continue
        out.append(c)
    return out
data=open(sys.argv[1],'rb').read()
for i,c in enumerate(pages(data)):
    if b'/Im' not in c and b' Do' not in c: continue
    txt=c.decode('latin-1')
    # track q/Q and cm
    import math
    ctm=[(1,0,0,1,0,0)]
    def mul(a,b):
        return (a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
                a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
                a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5])
    stack=[(1,0,0,1,0,0)]
    cur=(1,0,0,1,0,0)
    toks=re.findall(r'(-?\d*\.?\d+(?:e-?\d+)?)|([A-Za-z\'"*]+)|(/[^\s/\[\]<>()]+)', txt)
    nums=[]
    lastname=None
    for n,op,name in toks:
        if n: nums.append(float(n)); continue
        if name: lastname=name; nums=[]; continue
        if op=='q': stack.append(cur)
        elif op=='Q':
            if stack: cur=stack.pop()
        elif op=='cm' and len(nums)>=6:
            cur=mul(tuple(nums[-6:]),cur)
        elif op=='Do':
            print(f"stream {i} {lastname} w={cur[0]:.4f} h={cur[3]:.4f} x={cur[4]:.4f} y={cur[5]:.4f} b={cur[1]:.4f} c={cur[2]:.4f}")
        nums=[]
