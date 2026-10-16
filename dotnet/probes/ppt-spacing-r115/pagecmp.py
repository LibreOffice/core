import sys, pymupdf
def lines(path):
    p=pymupdf.open(path); out=[]
    for pg in p:
        L=[]
        for b in pg.get_text("rawdict")["blocks"]:
            if b["type"]!=0: continue
            for l in b["lines"]:
                for s in l["spans"]:
                    ch=s["chars"]; t="".join(c["c"] for c in ch)
                    if not t.strip(): continue
                    L.append((round(ch[0]["origin"][0],2), round(ch[0]["origin"][1],2), t.replace(" ","")))
        out.append(sorted(L))
    return out
A=lines(sys.argv[1]); B=lines(sys.argv[2])
print(f"pages {len(A)} vs {len(B)}")
tot=0; movedpages=0
for i,(a,b) in enumerate(zip(A,B)):
    sa={t for _,_,t in a}; sb={t for _,_,t in b}
    da=dict(((t),(x,y)) for x,y,t in a); db=dict(((t),(x,y)) for x,y,t in b)
    common=sa&sb
    worst=0
    for t in common:
        worst=max(worst, abs(da[t][0]-db[t][0]), abs(da[t][1]-db[t][1]))
    diff = len(sa^sb)
    if diff or worst>0.5:
        movedpages+=1
        print(f"  p{i+1:2d} runs {len(a)}/{len(b)} sym-diff={diff} worst-common-origin-shift={worst:.2f}")
    tot+=diff
print("pages differing:", movedpages, " total run-set symmetric difference:", tot)
