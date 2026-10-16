#!/usr/bin/env python3
"""The characters each text-range hyperlink covers, with the shape's own text around them."""
import struct, sys, pathlib, olefile

TEXTCHARS, TEXTBYTES, TEXTHEADER = 4000, 4008, 3999
TXINTERACTIVE, INTERACTIVEINFO, INTERACTIVEINFOATOM = 4063, 4082, 4083
CLIENTTEXTBOX, SLIDELISTWITHTEXT = 0xF00D, 4080

def records(buf, s, e):
    p = s
    while p + 8 <= e:
        vi, rt, rl = struct.unpack_from('<HHI', buf, p)
        body, stop = p + 8, p + 8 + rl
        if stop > e: return
        yield (vi & 0x0F, rt, body, stop, vi >> 4)
        p = stop

def scan(buf, s, e, container=None):
    seq = list(records(buf, s, e))
    text = None
    for ver, rt, b, st, inst in seq:
        if rt == TEXTCHARS: text = buf[b:st].decode('utf-16-le', 'replace')
        elif rt == TEXTBYTES: text = buf[b:st].decode('cp1252', 'replace')
    for i, (ver, rt, b, st, inst) in enumerate(seq):
        if rt == INTERACTIVEINFO:
            nxt = seq[i+1] if i+1 < len(seq) else None
            if nxt and nxt[1] == TXINTERACTIVE and nxt[3] - nxt[2] >= 8:
                a, z = struct.unpack_from('<II', buf, nxt[2])
                hid = None
                for v2, t2, b2, s2, _ in records(buf, b, st):
                    if t2 == INTERACTIVEINFOATOM:
                        hid = struct.unpack_from('<I', buf, b2 + 4)[0]
                if text is not None:
                    yield (hid, a, z, repr(text[max(0,a-12):a]), repr(text[a:z]), repr(text[z:z+12]))
                else:
                    yield (hid, a, z, '<no text in this container>', '', '')
        elif ver == 0x0F:
            yield from scan(buf, b, st, rt)

for name in sys.argv[1:]:
    p = next(q for q in pathlib.Path('/home/user/sample-files').rglob('*') if q.name == name)
    ole = olefile.OleFileIO(str(p)); buf = ole.openstream('PowerPoint Document').read(); ole.close()
    print(f'== {name}')
    for row in scan(buf, 0, len(buf)):
        print('   id=%s [%d,%d) before=%s LINK=%s after=%s' % row)
