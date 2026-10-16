import sys, zipfile, os, collections, re
from xml.etree import ElementTree as ET
C="http://schemas.openxmlformats.org/drawingml/2006/chart"
root=sys.argv[1]
files=[]
for dp,dn,fn in os.walk(root):
    for f in fn:
        if f.lower().endswith((".docx",".docm",".dotx",".dotm",".xlsx",".xlsm",".xltx",".xltm",".pptx",".pptm",".potx",".potm",".ppsx",".ppsm")):
            files.append(os.path.join(dp,f))
seen=set(); uniq=[]
for f in sorted(files):
    k=os.path.realpath(f).casefold()
    if k in seen: continue
    seen.add(k); uniq.append(f)
multi=collections.Counter(); ofpie=collections.Counter(); ofpie_pct=collections.Counter()
multi_kind=collections.Counter()
for f in uniq:
    try: z=zipfile.ZipFile(f)
    except Exception: continue
    fam = f.split(os.sep)[len(root.rstrip(os.sep).split(os.sep))-1] if False else os.path.relpath(f,root).split(os.sep)[0]
    for p in z.namelist():
        if "chart" not in p or not p.endswith(".xml"): continue
        try: data=z.read(p)
        except Exception: continue
        if b"drawingml/2006/chart" not in data: continue
        try: r=ET.fromstring(data)
        except Exception: continue
        rel=os.path.relpath(f,root)
        has_multi = r.find(".//{%s}multiLvlStrRef"%C) is not None
        if has_multi:
            multi[rel]+=1
            # which plot kinds in this chart
            for e in r.iter():
                ln=e.tag.split("}")[-1]
                if ln.endswith("Chart") and ln!="chart":
                    multi_kind[ln]+=1
        for e in r.iter("{%s}ofPieChart"%C):
            ofpie[rel]+=1
            sp = e.find(".//{%s}showPercent"%C)
            if sp is not None and sp.get("val") in ("1","true"): ofpie_pct[rel]+=1
print("=== charts with multiLvlStrRef ===", len(multi))
for k,v in sorted(multi.items()): print(f"  {v}  {k}")
print("plot kinds present in those charts:", dict(multi_kind))
print("=== ofPieChart ===", len(ofpie))
for k,v in sorted(ofpie.items()): print(f"  {v}  {k}   showPercent={ofpie_pct.get(k,0)}")
