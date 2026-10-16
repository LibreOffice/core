import sys, zipfile, os, collections
from xml.etree import ElementTree as ET
VML="urn:schemas-microsoft-com:vml"; O="urn:schemas-microsoft-com:office:office"
A="http://schemas.openxmlformats.org/drawingml/2006/main"
WPS="http://schemas.microsoft.com/office/word/2010/wordprocessingShape"
PIC="http://schemas.openxmlformats.org/drawingml/2006/picture"
MC="http://schemas.openxmlformats.org/markup-compatibility/2006"
root=sys.argv[1]
files=[]
for dp,dn,fn in os.walk(root):
    for f in fn:
        if f.lower().endswith((".docx",".docm",".dotx",".dotm")): files.append(os.path.join(dp,f))
seen=set(); uniq=[]
for f in sorted(files):
    k=os.path.realpath(f).casefold()
    if k in seen: continue
    seen.add(k); uniq.append(f)
vmldocs=collections.Counter(); stydocs=collections.Counter(); zerodocs=collections.Counter()
for f in uniq:
    try: z=zipfile.ZipFile(f)
    except Exception: continue
    rel=os.path.relpath(f,root)
    for p in [n for n in z.namelist() if n.startswith("word/") and n.endswith(".xml")]:
        try: data=z.read(p)
        except Exception: continue
        try: r=ET.fromstring(data)
        except Exception: continue
        fb=set()
        for e0 in r.iter("{%s}Fallback"%MC):
            for e in e0.iter(): fb.add(id(e))
        stypes={st.get("id"):st.get("{%s}spt"%O) for st in r.iter("{%s}shapetype"%VML)}
        for el in r.iter():
            if el.tag.startswith("{%s}"%VML) and id(el) not in fb:
                ln=el.tag.split("}")[1]
                if ln in ("rect","roundrect") and (el.get("fillcolor") or el.get("strokecolor")):
                    vmldocs[rel]+=1
                if ln=="shape":
                    ty=(el.get("type") or "").lstrip("#")
                    spt=el.get("{%s}spt"%O) or stypes.get(ty)
                    if (el.get("{%s}connectortype"%O) or spt=="32") and el.get("strokecolor"):
                        vmldocs[rel]+=1
        # DrawingML shapes with a style ref and no own fill/line
        for wsp in list(r.iter("{%s}wsp"%WPS))+list(r.iter("{%s}sp"%A)):
            if id(wsp) in fb: continue
            spPr=wsp.find("{%s}spPr"%WPS) or wsp.find("{%s}spPr"%A)
            sty=wsp.find("{%s}style"%WPS) or wsp.find("{%s}style"%A)
            if sty is None: continue
            ownfill = spPr is not None and any(spPr.find("{%s}%s"%(A,k)) is not None for k in ("solidFill","noFill","gradFill","blipFill","pattFill","grpFill"))
            ownln = spPr is not None and spPr.find("{%s}ln"%A) is not None
            fr=sty.find("{%s}fillRef"%A); lr=sty.find("{%s}lnRef"%A)
            if (not ownfill and fr is not None and fr.get("idx") not in (None,"0")) or (not ownln and lr is not None and lr.get("idx") not in (None,"0")):
                stydocs[rel]+=1
            # zero extent
            if spPr is not None:
                xf=spPr.find("{%s}xfrm"%A)
                if xf is not None:
                    ext=xf.find("{%s}ext"%A)
                    if ext is not None and (ext.get("cx")=="0" or ext.get("cy")=="0"): zerodocs[rel]+=1
print("=== VML rect/connector with stated colour: %d documents ==="%len(vmldocs))
for k,v in sorted(vmldocs.items()): print(f"  {v:4d}  {k}")
print("=== DrawingML shapes taking fill/line from wps:style: %d documents ==="%len(stydocs))
for k,v in sorted(stydocs.items(), key=lambda x:-x[1])[:40]: print(f"  {v:4d}  {k}")
print("   total shapes:", sum(stydocs.values()))
print("=== zero-extent DrawingML members: %d documents ==="%len(zerodocs))
for k,v in sorted(zerodocs.items(), key=lambda x:-x[1])[:20]: print(f"  {v:4d}  {k}")
