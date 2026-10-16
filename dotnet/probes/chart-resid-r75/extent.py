import sys, pymupdf
def scan(path):
    d = pymupdf.open(path)
    out = []
    for i, pg in enumerate(d):
        w, h = pg.rect.width, pg.rect.height
        xmin=ymin=1e18; xmax=ymax=-1e18; n=0
        for dr in pg.get_drawings():
            for it in dr["items"]:
                pts=[]
                if it[0]=="l": pts=[it[1],it[2]]
                elif it[0]=="c": pts=list(it[1:5])
                elif it[0]=="re": r=it[1]; pts=[pymupdf.Point(r.x0,r.y0),pymupdf.Point(r.x1,r.y1)]
                elif it[0]=="qu":
                    q=it[1]; pts=[q.ul,q.ur,q.ll,q.lr]
                for p in pts:
                    n+=1
                    xmin=min(xmin,p.x); xmax=max(xmax,p.x)
                    ymin=min(ymin,p.y); ymax=max(ymax,p.y)
        if n:
            out.append((i+1, w, h, round(xmin,2), round(xmax,2), round(ymin,2), round(ymax,2), n))
    return out
for path in sys.argv[1:]:
    print("==", path)
    for r in scan(path):
        pg,w,h,x0,x1,y0,y1,n = r
        flag = "  OUT" if (x0 < -1 or y0 < -1 or x1 > w+1 or y1 > h+1) else ""
        print(f"p{pg:3d} page {w:.0f}x{h:.0f}  x {x0:12.2f}..{x1:12.2f}  y {y0:10.2f}..{y1:10.2f}  n={n}{flag}")
