#!/usr/bin/env python3
"""Exact advance width of a string from a TTF's own hmtx, in font units and in twips at a size."""
import struct

class Face:
    def __init__(self, path):
        d = self.d = open(path, 'rb').read()
        n = struct.unpack('>H', d[4:6])[0]
        self.t = {}
        for i in range(n):
            o = 12 + 16 * i
            tag = d[o:o + 4].decode('latin1')
            off, ln = struct.unpack('>II', d[o + 8:o + 16])
            self.t[tag] = (off, ln)
        self.upem = struct.unpack('>H', d[self.t['head'][0] + 18:self.t['head'][0] + 20])[0]
        self.numh = struct.unpack('>H', d[self.t['hhea'][0] + 34:self.t['hhea'][0] + 36])[0]
        self.hmtx = self.t['hmtx'][0]
        cm = self.t['cmap'][0]
        sub = None
        for i in range(struct.unpack('>H', d[cm + 2:cm + 4])[0]):
            pid, eid, off = struct.unpack('>HHI', d[cm + 4 + 8 * i:cm + 4 + 8 * i + 8])
            if (pid, eid) in ((3, 1), (3, 10), (0, 3), (0, 4)):
                sub = cm + off
        self.sub = sub
        self.segX2 = struct.unpack('>H', d[sub + 6:sub + 8])[0]

    def gid(self, ch):
        d, sub, segX2 = self.d, self.sub, self.segX2
        c = ord(ch); seg = segX2 // 2
        endo = sub + 14; starto = endo + segX2 + 2; deltao = starto + segX2; rangeo = deltao + segX2
        for i in range(seg):
            end = struct.unpack('>H', d[endo + 2 * i:endo + 2 * i + 2])[0]
            if c <= end:
                start = struct.unpack('>H', d[starto + 2 * i:starto + 2 * i + 2])[0]
                if c < start: return 0
                delta = struct.unpack('>h', d[deltao + 2 * i:deltao + 2 * i + 2])[0]
                ro = struct.unpack('>H', d[rangeo + 2 * i:rangeo + 2 * i + 2])[0]
                if ro == 0: return (c + delta) & 0xFFFF
                a = rangeo + 2 * i + ro + 2 * (c - start)
                g = struct.unpack('>H', d[a:a + 2])[0]
                return 0 if g == 0 else (g + delta) & 0xFFFF
        return 0

    def adv(self, gid):
        g = min(gid, self.numh - 1)
        return struct.unpack('>H', self.d[self.hmtx + 4 * g:self.hmtx + 4 * g + 2])[0]

    def units(self, s):
        return sum(self.adv(self.gid(c)) for c in s)

    def twips(self, s, size_pt):
        return self.units(s) * size_pt * 20.0 / self.upem

if __name__ == '__main__':
    import sys
    f = Face('/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf')
    for s in sys.argv[1:]:
        print(f'{s!r}\tunits {f.units(s)}\t{f.twips(s, 8.0):.4f} tw at 8 pt')
