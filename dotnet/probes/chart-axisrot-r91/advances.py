import struct, sys
path = "/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf"
d = open(path,'rb').read()
num = struct.unpack('>H', d[4:6])[0]
tabs = {}
for i in range(num):
    o = 12 + 16*i
    tag = d[o:o+4].decode('latin1')
    off, ln = struct.unpack('>II', d[o+8:o+16])
    tabs[tag] = (off, ln)
ho, _ = tabs['head']; upem = struct.unpack('>H', d[ho+18:ho+20])[0]
hh, _ = tabs['hhea']; nhm = struct.unpack('>H', d[hh+34:hh+36])[0]
hm, _ = tabs['hmtx']
adv = [struct.unpack('>H', d[hm+4*i:hm+4*i+2])[0] for i in range(nhm)]
# cmap format 4
co, _ = tabs['cmap']
n = struct.unpack('>H', d[co+2:co+4])[0]
sub = None
for i in range(n):
    pid, eid, off = struct.unpack('>HHI', d[co+4+8*i:co+12+8*i])
    if (pid, eid) in ((3,1),(0,3),(0,4),(3,10)): sub = co + off
fmt = struct.unpack('>H', d[sub:sub+2])[0]
assert fmt == 4, fmt
segX2 = struct.unpack('>H', d[sub+6:sub+8])[0]; seg = segX2//2
ends = struct.unpack('>%dH'%seg, d[sub+14:sub+14+segX2])
starts = struct.unpack('>%dH'%seg, d[sub+16+segX2:sub+16+2*segX2])
deltas = struct.unpack('>%dh'%seg, d[sub+16+2*segX2:sub+16+3*segX2])
robase = sub+16+3*segX2
ros = struct.unpack('>%dH'%seg, d[robase:robase+segX2])
def gid(ch):
    c = ord(ch)
    for i in range(seg):
        if c <= ends[i]:
            if c < starts[i]: return 0
            if ros[i] == 0: return (c + deltas[i]) & 0xFFFF
            p = robase + 2*i + ros[i] + 2*(c - starts[i])
            g = struct.unpack('>H', d[p:p+2])[0]
            return 0 if g == 0 else (g + deltas[i]) & 0xFFFF
    return 0
def A(g): return adv[g] if g < nhm else adv[-1]
SIZE = 11.0
SCALE = round(SIZE*96/72)/(SIZE*96/72)     # chart2's 96 dpi device: 15 px for a 14.667 em
print(f"upem={upem} pixel-em scale={SCALE:.6f}")
for w in ["Product","Quality","Innovation","Brand","Reputation","Cost","Efficiency",
          "Customer","Service","nnnnnnnn","nnnnnnnnn","nnnnnn"]:
    units = sum(A(gid(c)) for c in w)
    design = units*SIZE/upem
    print(f"  {w:12s} units={units:6d} design={design:7.3f}pt device={design*SCALE:7.3f}pt")
