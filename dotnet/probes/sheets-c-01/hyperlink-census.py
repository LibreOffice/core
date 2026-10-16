import olefile, struct, os, zipfile, re
root='/c/sandbox/workdir/sample-files/sheets'
rows=[]
for dp,dn,fn in os.walk(root):
    for f in sorted(fn):
        p=os.path.join(dp,f)
        n=0
        if zipfile.is_zipfile(p):
            try:
                z=zipfile.ZipFile(p)
                for name in z.namelist():
                    if re.match(r'xl/worksheets/sheet\d+\.xml$',name):
                        s=z.read(name).decode('utf-8','replace')
                        n+=len(re.findall(r'<hyperlink ',s))
                z.close()
            except Exception: pass
        elif olefile.isOleFile(p):
            try:
                o=olefile.OleFileIO(p)
                ents=[e for e in o.listdir() if e[-1] in ('Workbook','Book')]
                if ents:
                    data=o.openstream(ents[0]).read()
                    i=0
                    while i+4<=len(data):
                        rid,ln=struct.unpack_from('<HH',data,i); i+=4; i+=ln
                        if rid==0x01B8: n+=1
                o.close()
            except Exception: pass
        if n: rows.append((f,n))
print("documents with hyperlink records:", len(rows))
for f,n in sorted(rows,key=lambda r:-r[1]): print(f"{n:6d}  {f}")
