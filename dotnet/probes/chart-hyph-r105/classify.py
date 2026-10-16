import sys, glob, os, math, pymupdf
for path in sorted(glob.glob(sys.argv[1] + "/*.pdf")):
    d = pymupdf.open(path); p = d[0]
    lines=[]
    for b in p.get_text('dict')['blocks']:
        if b['type']!=0: continue
        for l in b['lines']:
            t=''.join(s['text'] for s in l['spans'])
            lines.append((t, l['dir']))
    # category labels: any line containing one of the target tokens
    toks = ('Cost','Effic','Stretch','ffi','tretch','-')
    cat=[(t,dr) for t,dr in lines if any(k in t for k in toks)]
    rot = any(abs(dr[1])>1e-6 for t,dr in cat)
    nfills = 0
    for dr in p.get_drawings():
        if dr['fill'] is None: continue
        r=dr['rect']
        if 0.3<r.width<15 and 0.3<r.height<15: nfills+=1
    if not cat:
        v = f"ROTATED-outlined (fills={nfills})"
    elif rot:
        v = "ROTATED-text"
    else:
        v = f"upright, {len(cat)} lines: " + " | ".join(sorted({t for t,_ in cat}))
    print(f"{os.path.basename(path):22s} {v}")
    d.close()
