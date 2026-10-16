import struct, sys

def tables(path):
    d = open(path,'rb').read()
    ver, num = struct.unpack('>IH', d[0:6])
    t = {}
    for i in range(num):
        off = 12 + 16*i
        tag = d[off:off+4].decode('latin1')
        o, l = struct.unpack('>II', d[off+8:off+16])
        t[tag] = d[o:o+l]
    return t

def metrics(path):
    t = tables(path)
    head = t['head']; upem = struct.unpack('>H', head[18:20])[0]
    hhea = t['hhea']
    asc, desc, gap = struct.unpack('>hhh', hhea[4:10])
    os2 = t['OS/2']
    ver = struct.unpack('>H', os2[0:2])[0]
    yStrikeoutSize, yStrikeoutPosition = struct.unpack('>hh', os2[26:30])
    fsSelection = struct.unpack('>H', os2[62:64])[0]
    typoAsc, typoDesc, typoGap = struct.unpack('>hhh', os2[68:74])
    winAsc, winDesc = struct.unpack('>HH', os2[74:78])
    post = t['post']
    ulPos, ulThick = struct.unpack('>hh', post[8:12])
    return dict(upem=upem, hheaAsc=asc, hheaDesc=desc, hheaGap=gap,
                typoAsc=typoAsc, typoDesc=typoDesc, typoGap=typoGap,
                winAsc=winAsc, winDesc=winDesc, fsSelection=fsSelection,
                useTypo=bool(fsSelection & (1<<7)),
                strikeSize=yStrikeoutSize, strikePos=yStrikeoutPosition,
                ulPos=ulPos, ulThick=ulThick)

if __name__ == '__main__':
    for p in sys.argv[1:]:
        m = metrics(p)
        print(p.split('/')[-1], m)
