import sys, pymupdf
d=pymupdf.open(sys.argv[1]); pg=d[int(sys.argv[2])-1]; L=float(sys.argv[3])
rows={}
for b in pg.get_text("rawdict")["blocks"]:
    if b["type"]!=0: continue
    for l in b["lines"]:
        for s in l["spans"]:
            ch=s["chars"]
            t="".join(c["c"] for c in ch)
            if not t.strip(): continue
            y=round(ch[0]["origin"][1],1)
            rows.setdefault(y,[]).append((ch[0]["origin"][0], t, ch))
for y in sorted(rows):
    parts=sorted(rows[y])
    if abs(parts[0][0]-L)>0.06: continue
    txt="".join(p[1] for p in parts)
    chs=[c for p in parts for c in p[2]]
    # last non-space char's advance end
    k=len(chs)-1
    while k>=0 and chs[k]["c"].isspace(): k-=1
    print("y=%7.2f  end(nonspace)=%7.2f  n=%2d  %s" % (y, chs[k]["bbox"][2], len(chs), repr(txt)))
