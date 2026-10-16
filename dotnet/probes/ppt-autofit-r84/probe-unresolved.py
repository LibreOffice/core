#!/usr/bin/env python3
"""The three documents whose Tx atoms resolve to nothing: what do they state?"""
import struct, olefile, sys, pathlib
sys.path.insert(0, '.')
EXOBJLIST, EXHYPERLINKATOM, EXHYPERLINK = 1033, 4051, 4055
TXINTERACTIVE, INTERACTIVEINFO, INTERACTIVEINFOATOM = 4063, 4082, 4083
CLIENTTEXTBOX = 0xF00D

def records(buf, start, end):
    p = start
    while p + 8 <= end:
        vi, rt, rl = struct.unpack_from('<HHI', buf, p)
        body, stop = p + 8, p + 8 + rl
        if stop > end: return
        yield (vi & 0x0F, rt, body, stop)
        p = stop

def walk(buf, s, e, chain=()):
    for ver, rt, b, st in records(buf, s, e):
        yield (rt, b, st, chain)
        if ver == 0x0F:
            yield from walk(buf, b, st, chain + (rt,))

for name in sys.argv[1:]:
    p = next(q for q in pathlib.Path('/home/user/sample-files').rglob('*') if q.name == name)
    ole = olefile.OleFileIO(str(p))
    buf = ole.openstream('PowerPoint Document').read()
    streams = ['/'.join(s) for s in ole.listdir()]
    ole.close()
    ids, exhl, parents = [], 0, {}
    for rt, b, st, chain in walk(buf, 0, len(buf)):
        if rt == EXHYPERLINKATOM: ids.append(struct.unpack_from('<I', buf, b)[0])
        if rt == EXHYPERLINK: exhl += 1
        if rt == EXOBJLIST: parents['exobjlist'] = parents.get('exobjlist', 0) + 1
    hids = []
    def scan(s, e, chain=()):
        seq = list(records(buf, s, e))
        for i, (ver, rt, b, st) in enumerate(seq):
            if rt == INTERACTIVEINFO:
                hid = None
                for t2, b2, s2, _ in walk(buf, b, st):
                    if t2 == INTERACTIVEINFOATOM and s2 - b2 >= 16:
                        sound, hyper = struct.unpack_from('<II', buf, b2)
                        action, oleverb, jump, flags, htype = struct.unpack_from('<BBBBB', buf, b2 + 8)
                        hid = (hyper, action, jump, flags, htype)
                        break
                nxt = seq[i+1] if i+1 < len(seq) else None
                if hid and nxt and nxt[1] == TXINTERACTIVE:
                    rng = struct.unpack_from('<II', buf, nxt[2])
                    hids.append((hid, rng, chain))
            elif ver == 0x0F:
                scan(b, st, chain + (rt,))
    scan(0, len(buf))
    from collections import Counter
    print(f'== {name}')
    print(f'   streams: {[s for s in streams if "Hlink" in s or "Summary" in s]}')
    print(f'   ExObjList {parents.get("exobjlist",0)}  ExHyperlink {exhl}  ExHyperlinkAtom ids {ids[:10]}')
    print(f'   Tx pairs {len(hids)}; ids {Counter(h[0][0] for h in hids).most_common(6)}')
    print(f'   actions {Counter((h[0][1], h[0][4]) for h in hids).most_common(6)}')
    print(f'   parent chains {Counter(tuple(hex(c) for c in h[2]) for h in hids).most_common(3)}')
    print(f'   sample ranges {[h[1] for h in hids[:6]]}')
