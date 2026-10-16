#!/usr/bin/env python3
"""How many BIFF shapes set fAutoTextMargin, and how many are grouped, per workbook."""
import sys, olefile, struct
def sheets(path):
    ole=olefile.OleFileIO(path)
    ent=[e for e in ole.listdir() if e[-1].lower() in ("workbook","book")]
    if not ent: return []
    data=ole.openstream(ent[0]).read()
    pos=0; recs=[]
    while pos+4<=len(data):
        rid,rlen=struct.unpack_from("<HH",data,pos); recs.append((pos,rid,rlen)); pos+=4+rlen
    BOF=0x0809; EOFR=0x000A; MSO=0x00EC; CONT=0x003C
    out=[]; cur=None; depth=0; prev=None
    for (pos,rid,rlen) in recs:
        if rid==BOF:
            depth+=1
            cur={"dff":bytearray(),"depth":depth}; out.append(cur); prev=rid; continue
        if rid==EOFR: depth-=1; prev=rid; continue
        if cur is None: continue
        if rid==MSO and depth==cur["depth"]:
            cur["dff"]+=data[pos+4:pos+4+rlen]; prev=MSO
        elif rid==CONT and prev==MSO and depth==cur["depth"]:
            cur["dff"]+=data[pos+4:pos+4+rlen]
        elif rid!=CONT: prev=rid
    return [bytes(s["dff"]) for s in out if s["dff"]]

def walk(buf,start,end,depth,out):
    p=start
    while p+8<=end:
        vi,typ,ln=struct.unpack_from("<HHI",buf,p)
        body=p+8; be=body+ln
        out.append((depth,typ,ln,body,min(be,end),vi>>4))
        if (vi&0xF)==0xF: walk(buf,body,min(be,end),depth+1,out)
        p=be
        if ln==0 and typ==0: break
    return out

def opt_props(buf,s,e,inst):
    """(id -> value) from an msofbtOPT/UDefProp block."""
    props={}
    p=s
    for i in range(inst):
        if p+6>e: break
        pid,val=struct.unpack_from("<HI",buf,p); p+=6
        props[pid & 0x3FFF]=val
    return props

tot_shapes=tot_auto=tot_group=tot_child=0
for path in sys.argv[1:]:
    try: strms=sheets(path)
    except Exception: continue
    shapes=auto=grp=child=0
    for buf in strms:
        out=walk(buf,0,len(buf),0,[])
        # each F004 container: find its F00B (OPT) and F00A (Sp)
        for k,(d,t,ln,s,e,inst) in enumerate(out):
            if t!=0xF004: continue
            shapes+=1
            kids=[x for x in out if x[0]==d+1 and s<=x[3]<e]
            props={}
            flags=0
            haschild=False
            for (dd,tt,ll,ss,ee,ii) in kids:
                if tt==0xF00B: props.update(opt_props(buf,ss,ee,ii))
                elif tt==0xF00A and ee-ss>=8: flags=struct.unpack_from("<I",buf,ss+4)[0]
                elif tt==0xF00F: haschild=True
            if flags & 0x0001: grp+=1     # msospgr / group flag
            if haschild: child+=1
            v=props.get(191,0)
            if v & (1 << (191-188)): auto+=1
    tot_shapes+=shapes; tot_auto+=auto; tot_group+=grp; tot_child+=child
    if shapes: print(f"{shapes}\t{auto}\t{grp}\t{child}\t{path.split('/')[-1]}")
print(f"TOTAL shapes={tot_shapes} autoMargin={tot_auto} groupFlag={tot_group} childAnchor={tot_child}")
