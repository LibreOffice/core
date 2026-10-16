import sys, zipfile, os, collections
from xml.etree import ElementTree as ET
VML="urn:schemas-microsoft-com:vml"; W="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
MC="http://schemas.openxmlformats.org/markup-compatibility/2006"; O="urn:schemas-microsoft-com:office:office"
files=[]
for dp,dn,fn in os.walk(sys.argv[1]):
    for f in fn:
        if f.lower().endswith((".docx",".docm",".dotx",".dotm")): files.append(os.path.join(dp,f))
seen=set(); uniq=[]
for f in sorted(files):
    k=os.path.realpath(f).casefold()
    if k in seen: continue
    seen.add(k); uniq.append(f)
t=collections.Counter(); docs=collections.defaultdict(collections.Counter)
for f in uniq:
    try: z=zipfile.ZipFile(f)
    except Exception: continue
    for p in [n for n in z.namelist() if n.startswith("word/") and n.endswith(".xml")]:
        try: data=z.read(p)
        except Exception: continue
        if b"urn:schemas-microsoft-com:vml" not in data: continue
        try: root=ET.fromstring(data)
        except Exception: continue
        fb=set()
        for e0 in root.iter("{%s}Fallback"%MC):
            for e in e0.iter(): fb.add(id(e))
        # shapetype id -> its spt
        stypes={}
        for st in root.iter("{%s}shapetype"%VML):
            stypes[st.get("id")] = st.get("{%s}spt"%O)
        for el in root.iter():
            if not el.tag.startswith("{%s}"%VML): continue
            ln=el.tag.split("}")[1]
            if ln not in ("shape","rect","roundrect","oval","group","line","polyline","curve","arc","background"): continue
            if id(el) in fb: continue
            ty=el.get("type","")
            spt=el.get("{%s}spt"%O) or stypes.get(ty.lstrip("#"),"")
            conn=el.get("{%s}connectortype"%O)
            key=f"{ln} type={ty} spt={spt} conn={conn}"
            has_img = el.find("{%s}imagedata"%VML) is not None
            has_tx = any(True for _ in el.iter("{%s}txbxContent"%W))
            key += f" img={int(has_img)} tx={int(has_tx)} fc={int(el.get('fillcolor') is not None)} sc={int(el.get('strokecolor') is not None)}"
            t[key]+=1
            docs[key][os.path.basename(f)]+=1
for k,v in sorted(t.items(), key=lambda x:-x[1]):
    dd=", ".join(f"{d}:{c}" for d,c in docs[k].most_common(4))
    print(f"{v:5d}  {k}   [{dd}]")
