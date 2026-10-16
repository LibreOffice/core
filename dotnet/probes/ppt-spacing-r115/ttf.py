import struct
class TTF:
    def __init__(self, path):
        d=open(path,'rb').read(); self.d=d
        n=struct.unpack('>H', d[4:6])[0]
        self.t={}
        for i in range(n):
            o=12+16*i
            tag=d[o:o+4].decode('latin-1')
            off,ln=struct.unpack('>II', d[o+8:o+16])
            self.t[tag]=(off,ln)
        ho,_=self.t['head']; self.upem=struct.unpack('>H', d[ho+18:ho+20])[0]
        hh,_=self.t['hhea']; self.numh=struct.unpack('>H', d[hh+34:hh+36])[0]
        self.hm=self.t['hmtx'][0]
        self._cmap()
    def _cmap(self):
        d=self.d; co,_=self.t['cmap']
        n=struct.unpack('>H', d[co+2:co+4])[0]
        best=None
        for i in range(n):
            pid,eid,off=struct.unpack('>HHI', d[co+4+8*i:co+12+8*i])
            if (pid,eid) in ((3,1),(3,10),(0,3),(0,4),(0,6)): best=co+off
        assert best
        fmt=struct.unpack('>H', d[best:best+2])[0]
        self.map={}
        if fmt==4:
            segX2=struct.unpack('>H', d[best+6:best+8])[0]; seg=segX2//2
            E=best+14; S=E+segX2+2; D=S+segX2; R=D+segX2
            for i in range(seg):
                end=struct.unpack('>H', d[E+2*i:E+2*i+2])[0]
                sta=struct.unpack('>H', d[S+2*i:S+2*i+2])[0]
                delta=struct.unpack('>h', d[D+2*i:D+2*i+2])[0]
                ro=struct.unpack('>H', d[R+2*i:R+2*i+2])[0]
                for c in range(sta, min(end,0xFFFF)+1):
                    if ro==0: g=(c+delta)&0xFFFF
                    else:
                        gi=R+2*i+ro+2*(c-sta)
                        if gi+2>len(d): continue
                        g=struct.unpack('>H', d[gi:gi+2])[0]
                        if g: g=(g+delta)&0xFFFF
                    if g: self.map[c]=g
        elif fmt==12:
            ng=struct.unpack('>I', d[best+12:best+16])[0]
            for i in range(ng):
                s,e,gs=struct.unpack('>III', d[best+16+12*i:best+28+12*i])
                for c in range(s,e+1): self.map[c]=gs+(c-s)
    def adv(self, ch):
        g=self.map.get(ord(ch))
        if g is None: return None
        i=min(g, self.numh-1)
        return struct.unpack('>H', self.d[self.hm+4*i:self.hm+4*i+2])[0]/self.upem
