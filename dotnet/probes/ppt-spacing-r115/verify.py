import sys, pymupdf
sys.path.insert(0,'/home/user/r115-work')
from ttf import TTF
F={'LiberationSans-Bold':'/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf',
   'LiberationSans':'/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf'}
D={'LiberationSans-Bold':'/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf',
   'LiberationSans':'/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'}
lib={k:TTF(v) for k,v in F.items()}; dej={k:TTF(v) for k,v in D.items()}
pdf=pymupdf.open(sys.argv[1]); pg=pdf[int(sys.argv[2])-1]
nL=nD=n=0; sL=sD=0.0; worstD=0; worstDc=None
for b in pg.get_text("rawdict")["blocks"]:
    if b["type"]!=0: continue
    for l in b["lines"]:
        for s in l["spans"]:
            f=s["font"]
            if f not in lib or abs(s["size"]-14.003)>0.01: continue
            ch=s["chars"]
            for i in range(len(ch)-1):
                c=ch[i]["c"]; nxt=ch[i+1]["c"]
                if c==' ' or nxt==' ': continue      # justification lives in the space
                adv=(ch[i+1]["origin"][0]-ch[i]["origin"][0])/s["size"]
                a=lib[f].adv(c); d=dej[f].adv(c)
                if a is None or d is None: continue
                n+=1; sL+=abs(adv-a); sD+=abs(adv-d)
                if abs(adv-a)<abs(adv-d): nL+=1
                else: nD+=1
                if abs(adv-d)>worstD: worstD=abs(adv-d); worstDc=(c,adv,d,a)
print(f"{sys.argv[3]:14s} n={n:4d}  closer-to-Liberation={nL:4d}  closer-to-DejaVu={nD:4d}  "
      f"mean|d-Lib|={sL/n:.5f}em  mean|d-DejaVu|={sD/n:.5f}em  worst-vs-DejaVu={worstD:.5f} {worstDc}")
