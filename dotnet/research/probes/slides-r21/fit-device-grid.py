import math, re
from collections import Counter
MM=2540.0/72.0
txt=open('dense.txt').read()
rows=[]
for b in txt.split('=== ')[1:]:
    cols=re.findall(r'column x~\s*([\d.]+)\s+sizes\(mm100\) \[([^\]]*)\]  n=(\d+)\n    pitches\(mm100\) \[([^\]]*)\]', b)
    no=af=None
    for x,sizes,n,p in cols:
        ps=[int(v) for v in p.split(', ') if v]; szs=[int(v) for v in sizes.split(', ') if v]
        if float(x)<200: no=(szs,ps)
        else: af=(szs,ps)
    if not no or not af or int(n)!=6: pass
    if not no or not af: continue
    em=max(af[0]); ba=Counter(af[1]).most_common(1)[0][0]; bn=Counter(no[1]).most_common(1)[0][0]
    rows.append((em,bn,ba))
rows=rows[:53]
def fr(v): return math.floor(v+0.5)
best=[]
for d10 in range(300, 40000):
    dpi=d10/10.0
    ok=0
    for em,bn,ba in rows:
        px=fr(em*dpi/2540.0)
        E=fr(px*2540.0/dpi)
        if fr(E*1.2)==ba: ok+=1
    best.append((ok,dpi))
best.sort(reverse=True)
print("pixel round-trip model, best dpi:", best[:6], "of", len(rows))
