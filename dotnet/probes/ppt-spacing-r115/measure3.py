import sys, pymupdf
sys.path.insert(0,'/home/user/r115-work')
from ttf import TTF
F={'LiberationSans-Bold':'/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf',
   'LiberationSans':'/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf',
   'LiberationSans-Italic':'/usr/share/fonts/truetype/liberation/LiberationSans-Italic.ttf'}
lib={k:TTF(v) for k,v in F.items()}
p=pymupdf.open(sys.argv[1]); pg=p[int(sys.argv[2])-1]; L=float(sys.argv[3])
rows={}
for b in pg.get_text("rawdict")["blocks"]:
    if b["type"]!=0: continue
    for l in b["lines"]:
        for s in l["spans"]:
            ch=s["chars"]
            if not "".join(c["c"] for c in ch).strip(): continue
            y=round(ch[0]["origin"][1],1)
            r=rows.setdefault(y,[1e9,-1e9])
            r[0]=min(r[0], ch[0]["origin"][0])
            f=s["font"]; last=ch[-1]
            a=lib[f].adv(last["c"]) if f in lib else None
            end=last["origin"][0]+(a*s["size"] if a else 0)
            r[1]=max(r[1], end)
ends=[rows[y][1] for y in sorted(rows) if abs(rows[y][0]-L)<0.05]
ends=[e for e in ends if e>L+200]
print(f"{sys.argv[4]:14s} p{sys.argv[2]:>2s} left={L} n={len(ends)} ends: " + " ".join(f"{e:.2f}" for e in ends))
