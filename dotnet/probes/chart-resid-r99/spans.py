import sys, pymupdf
doc = pymupdf.open(sys.argv[1]); pg = doc[int(sys.argv[2])-1]
for b in pg.get_text("dict")["blocks"]:
    for l in b.get("lines", []):
        for s in l["spans"]:
            x0,y0,x1,y1 = s["bbox"]
            print(f"{x0:9.2f} {y0:8.2f} {x1:9.2f} {y1:8.2f} {s['size']:5.2f} {l['dir'][0]:4.1f} {s['text']!r}")
