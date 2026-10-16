"""Every 45-degree text line the reference draws, over the whole banked reference gate.

chart2 rotates a crowded axis to exactly 45 degrees (VCartesianAxis's ladder); a document
author can also state any angle.  Isolating 44.8-45.2 separates the automatic rotation from
stated ones, and gives the ceiling on how many corpus documents can be showing this at all.
"""
import pymupdf, math, glob, os, sys, json
out={}
for p in sorted(glob.glob("/home/user/gate-orig-r83/ref/*.pdf")):
    n=os.path.basename(p)
    try: d=pymupdf.open(p)
    except Exception: continue
    a45=0; other=0; samples=[]
    for page in d:
        for b in page.get_text('dict')['blocks']:
            if b['type']!=0: continue
            for l in b['lines']:
                dx,dy=l['dir']
                if abs(dy)<=0.05 or abs(abs(dy)-1.0)<=0.05: continue
                ang=math.degrees(math.atan2(-dy,dx))
                if 44.8<=ang<=45.2:
                    a45+=1
                    if len(samples)<3: samples.append(''.join(s['text'] for s in l['spans']))
                else: other+=1
    d.close()
    if a45 or other: out[n]=dict(a45=a45, other=other, samples=samples)
json.dump(out, open("ref45.json","w"), indent=0)
print("documents with any turned text:", len(out))
print("documents with 45-degree text:", sum(1 for v in out.values() if v['a45']))
