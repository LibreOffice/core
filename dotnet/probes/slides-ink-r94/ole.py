import struct, sys

def read_ole(path):
    d = open(path,'rb').read()
    assert d[:8] == b'\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1', 'not OLE2'
    ssz = 1 << struct.unpack_from('<H', d, 30)[0]
    sssz = 1 << struct.unpack_from('<H', d, 32)[0]
    nfat = struct.unpack_from('<I', d, 44)[0]
    dirstart = struct.unpack_from('<I', d, 48)[0]
    minicut = struct.unpack_from('<I', d, 56)[0]
    ministart = struct.unpack_from('<I', d, 60)[0]
    difstart = struct.unpack_from('<I', d, 68)[0]
    ndif = struct.unpack_from('<I', d, 72)[0]
    def sect(n): return d[512+n*ssz:512+(n+1)*ssz]
    fatsects = [struct.unpack_from('<I', d, 76+4*i)[0] for i in range(109)]
    nxt = difstart
    for _ in range(ndif):
        s = sect(nxt)
        fatsects += [struct.unpack_from('<I', s, 4*i)[0] for i in range(ssz//4 - 1)]
        nxt = struct.unpack_from('<I', s, ssz-4)[0]
    fat = []
    for fs in fatsects[:nfat]:
        s = sect(fs)
        fat += [struct.unpack_from('<I', s, 4*i)[0] for i in range(ssz//4)]
    def chain(start):
        out=[]; c=start
        while c < 0xFFFFFFFA and len(out) < 1_000_000:
            out.append(c); c = fat[c]
        return out
    dirdata = b''.join(sect(s) for s in chain(dirstart))
    entries=[]
    for i in range(0, len(dirdata), 128):
        e = dirdata[i:i+128]
        if len(e) < 128: break
        nlen = struct.unpack_from('<H', e, 64)[0]
        name = e[:max(0,nlen-2)].decode('utf-16-le', 'replace')
        typ = e[66]
        start = struct.unpack_from('<I', e, 116)[0]
        size = struct.unpack_from('<Q', e, 120)[0] if ssz>512 else struct.unpack_from('<I', e, 120)[0]
        entries.append((name, typ, start, size))
    root = entries[0]
    minifatsects = chain(struct.unpack_from('<I', d, 60)[0])
    minifat=[]
    for fs in chain(ministart):
        s = sect(fs)
        minifat += [struct.unpack_from('<I', s, 4*i)[0] for i in range(ssz//4)]
    ministream = b''.join(sect(s) for s in chain(root[2]))
    def read(name):
        for n,t,st,sz in entries:
            if n == name:
                if sz < minicut and n != 'Root Entry':
                    out=b''; c=st
                    while c < 0xFFFFFFFA:
                        out += ministream[c*sssz:(c+1)*sssz]; c = minifat[c]
                    return out[:sz]
                return b''.join(sect(s) for s in chain(st))[:sz]
        return None
    return entries, read
