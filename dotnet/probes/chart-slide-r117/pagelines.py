import sys, pymupdf
def lines(path, pno):
    d=pymupdf.open(path); pg=d[pno-1]; out=[]
    for b in pg.get_text("rawdict")["blocks"]:
        if b["type"]!=0: continue
        for l in b["lines"]:
            t="".join(c["c"] for s in l["spans"] for c in s["chars"])
            if not t.strip(): continue
            out.append((round(l["bbox"][1],1), round(l["bbox"][0],1), t.rstrip()))
    return sorted(out)
a=lines(sys.argv[1], int(sys.argv[3])); b=lines(sys.argv[2], int(sys.argv[3]))
import difflib
sa=[f"{y:7.1f} {x:7.1f} {t}" for y,x,t in a]
sb=[f"{y:7.1f} {x:7.1f} {t}" for y,x,t in b]
for line in difflib.unified_diff(sa, sb, "REF", "OURS", lineterm="", n=1):
    print(line)
