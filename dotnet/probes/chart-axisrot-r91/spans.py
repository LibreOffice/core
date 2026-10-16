import sys, glob, os, pymupdf
for path in sorted(glob.glob(sys.argv[1] + "/*.pdf")):
    p = pymupdf.open(path)[0]
    out=[]
    for b in p.get_text('dict')['blocks']:
        if b['type']!=0: continue
        for l in b['lines']:
            for s in l['spans']:
                t=s['text'].strip()
                if not t: continue
                bb=s['bbox']
                if bb[0]<400 or bb[1]<330 or bb[3]>460: continue
                if s['size']>11.6 or s['size']<10.4: continue
                out.append((round(bb[1],1), round(bb[0],2), round(bb[2]-bb[0],2),
                            round(l['dir'][1],3), t))
    out.sort()
    fills=[dr for dr in p.get_drawings() if dr['fill'] is not None]
    glyphs=[dr for dr in fills if dr['rect'].width<15 and dr['rect'].height<15]
    rows = sorted({y for y,_,_,_,_ in out})
    rot = any(abs(d)>1e-6 for _,_,_,d,_ in out)
    kind = 'ROTATED-outlined' if not out else ('ROTATED' if rot else ('WRAPPED %d rows'%len(rows) if len(rows)>1 else 'upright'))
    print(f"== {os.path.basename(path)[:-4]:12s} {kind:17s} spans={len(out)} glyphfills={len(glyphs)}")
    for y,x,w,d,t in out[:12]:
        print(f"     y={y:7.1f} x={x:7.2f} w={w:6.2f} {t!r}")
