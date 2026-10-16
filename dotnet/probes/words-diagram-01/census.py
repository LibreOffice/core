import subprocess, zipfile, os, collections, re
root="/home/user/sample-files"
files=[f.decode() for f in subprocess.run(["git","-C",root,"ls-files","-z"],capture_output=True).stdout.split(b"\0") if f]
docs=[f for f in files if f.split("/")[0] in ("words","sheets","slides")]
DGM=b"http://schemas.openxmlformats.org/drawingml/2006/diagram"
DSPSP=b"<dsp:sp"
rows=[]
for rel in docs:
    p=os.path.join(root,rel); fam=rel.split("/")[0]
    try: z=zipfile.ZipFile(p)
    except Exception: continue
    names=z.namelist()
    data=[n for n in names if "/diagrams/data" in n]
    if not data: 
        z.close(); continue
    draw=[n for n in names if "/diagrams/drawing" in n]
    usable=0; empty=0; shapes=0
    for n in draw:
        d=z.read(n)
        c=d.count(b"<dsp:sp ")+d.count(b"<dsp:sp>")
        if c>0: usable+=1; shapes+=c
        else: empty+=1
    # how many anchors name the diagram uri
    anchors=0
    for n in names:
        if n.endswith(".xml") and ("document.xml" in n or "/slides/" in n or "header" in n or "footer" in n or "drawing" in n and "/diagrams/" not in n):
            try: d=z.read(n)
            except Exception: continue
            anchors+=d.count(b'uri="'+DGM+b'"')
    rows.append((fam, rel, len(data), len(draw), usable, empty, shapes, anchors))
    z.close()
fam=collections.Counter(r[0] for r in rows)
print("documents with a dgm data part:", len(rows), dict(fam))
print("total data parts:", sum(r[2] for r in rows))
print("total drawing parts:", sum(r[3] for r in rows), " usable:", sum(r[4] for r in rows), " emptied/none:", sum(r[5] for r in rows))
print("total baked dsp:sp:", sum(r[6] for r in rows))
print("anchors naming the diagram uri:", sum(r[7] for r in rows))
print()
print("fam\tdata\tdrawing\tusable\tshapes\tanchors\tpath")
for r in sorted(rows):
    print("%s\t%d\t%d\t%d\t%d\t%d\t%s"%(r[0],r[2],r[3],r[4],r[6],r[7],r[1]))
