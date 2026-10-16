import olefile, struct, os, sys
root='/c/sandbox/workdir/sample-files/sheets'
tot=0; ole=0; withchart=0; rows=[]
for dp,dn,fn in os.walk(root):
    for f in sorted(fn):
        p=os.path.join(dp,f); tot+=1
        if not olefile.isOleFile(p): continue
        ole+=1
        try:
            o=olefile.OleFileIO(p)
            ents=[e for e in o.listdir() if e[-1] in ('Workbook','Book')]
            if not ents: o.close(); continue
            data=o.openstream(ents[0]).read(); o.close()
        except Exception as e:
            continue
        i=0; charts=0; dateax=0; labelrange=0; freq1=0; inchart=False
        while i+4<=len(data):
            rid,ln=struct.unpack_from('<HH',data,i); i+=4
            pl=data[i:i+ln]; i+=ln
            if rid==0x0809 and len(pl)>=6:
                dt=struct.unpack_from('<H',pl,2)[0]
                inchart = (dt==0x0020)
                if inchart: charts+=1
            elif rid==0x1020 and len(pl)>=8:
                labelrange+=1
                if struct.unpack_from('<H',pl,2)[0]==1: freq1+=1
            elif rid==0x1062 and len(pl)>=18:
                fl=struct.unpack_from('<H',pl,16)[0]
                if fl & 0x0010: dateax+=1
        if charts: 
            withchart+=1
            rows.append((f,charts,labelrange,freq1,dateax))
print(f"files={tot} ole2={ole} with chart substreams={withchart}")
print(f"{'file':60s} charts labelRange freq1 dateAxis")
for r in sorted(rows,key=lambda r:-r[4]):
    print(f"{r[0][:60]:60s} {r[1]:6d} {r[2]:10d} {r[3]:5d} {r[4]:8d}")
