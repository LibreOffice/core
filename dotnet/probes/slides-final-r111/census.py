import struct, olefile, sys, pathlib
def walk(buf, off, end, out, d=0):
    while off + 8 <= end:
        vi, rt, rl = struct.unpack_from('<HHI', buf, off)
        ver = vi & 0xF; inst = vi >> 4
        body, stop = off+8, min(off+8+rl, end)
        if rt in (4013, 2041):
            out.append((rt, inst, body, stop))
        if (ver == 0xF or rt in (5000,5002,5003)) and d < 12:
            walk(buf, body, stop, out, d+1)
        off = body + rl
def levels(buf, body, stop):
    depth = struct.unpack_from('<H', buf, body)[0]
    p = body+2; res=[]
    for i in range(min(depth,10)):
        if p+4>stop: break
        mask = struct.unpack_from('<I', buf, p)[0]; p+=4
        blip=0xFFFF; anm=0; sch=0
        if mask & 0x00800000: blip=struct.unpack_from('<H',buf,p)[0]; p+=2
        if mask & 0x02000000: anm=struct.unpack_from('<H',buf,p)[0]; p+=2
        if mask & 0x01000000: sch=struct.unpack_from('<I',buf,p)[0]; p+=4
        if mask & 0x04000000: p+=4
        if p+4>stop: break
        cm=struct.unpack_from('<I',buf,p)[0]; p+=4
        if cm & 0x00100000: p+=4
        res.append((mask,blip,anm,sch))
    return res
nb=na=0; docs_b=set(); docs_a=set(); tot=0
for f in sorted(pathlib.Path(sys.argv[1]).rglob('*')):
    if f.suffix.lower() not in ('.ppt','.pot','.pps'): continue
    tot+=1
    try:
        buf = olefile.OleFileIO(str(f)).openstream('PowerPoint Document').read()
    except Exception as e:
        print('ERR', f.name, e); continue
    out=[]; walk(buf,0,len(buf),out)
    gra = sum(1 for rt,_,_,_ in out if rt==2041)
    for rt,inst,body,stop in out:
        if rt!=4013: continue
        for mask,blip,anm,sch in levels(buf,body,stop):
            if blip != 0xFFFF: nb+=1; docs_b.add(f.name)
            if anm: na+=1; docs_a.add(f.name)
    if gra: print(f'{f.name}: bugra={gra}')
print('docs', tot, 'master levels with blip', nb, 'in', len(docs_b), 'docs:', sorted(docs_b))
print('master levels with hasAnm', na, 'in', len(docs_a), 'docs:', sorted(docs_a))
