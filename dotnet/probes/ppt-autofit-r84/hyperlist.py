#!/usr/bin/env python3
"""Simulate `ImplSdPPTImport::Import`'s `m_aHyperList`, entry for entry.

`sd/source/filter/ppt/pptin.cxx`:366-518 walks the `_PID_HLINKS` blob six properties at a
time -- four `VT_I4` and two strings through `PropItem::Read`
(`sd/source/filter/ppt/propread.cxx`:73-186) -- and **breaks out of the loop** the moment
one of the six does not read.  So the number of entries is not `nPropCount / 6`: it is how
far the blob parses.  :531-547 then gives entry *k* the *k*th `ExHyperlinkAtom` of the
`ExObjList`, and an entry past the last such record keeps `nIndex = 0`.
"""
import struct, sys, pathlib
import olefile, census
from persist import directory, current_user_edit

USERDEF = bytes([0x05,0xd5,0xcd,0xd5,0x9c,0x2e,0x1b,0x10,0x93,0x97,0x08,0x00,0x2b,0x2c,0xf9,0xae])
VT_I4, VT_LPSTR, VT_LPWSTR = 3, 30, 31

class Item:
    def __init__(self, buf, pos, end):
        self.b, self.p, self.e = buf, pos, end
    def u32(self):
        if self.p + 4 > self.e: return None
        v = struct.unpack_from('<I', self.b, self.p)[0]; self.p += 4; return v
    def string(self):
        start = self.p
        t = self.u32()
        n = self.u32()
        if t is None or n is None: self.p = start; return None
        if t == VT_LPSTR:
            if n == 0 or self.p + n > self.e: self.p = start; return None
            raw = self.b[self.p:self.p + n]; self.p += n
            ok = raw[-1] == 0
            self.p += (4 - (n & 3)) & 3
            if not ok: self.p = start; return None
            return raw.split(b'\0')[0].decode('latin-1')
        if t == VT_LPWSTR:
            if n == 0 or self.p + 2 * n > self.e: self.p = start; return None
            raw = self.b[self.p:self.p + 2 * n]; self.p += 2 * n
            ok = raw[-2:] == b'\0\0'
            if n & 1: self.p += 2
            if not ok: self.p = start; return None
            return raw.decode('utf-16-le', 'replace').split('\0')[0]
        self.p = start
        return None

def pid_hlinks_blob(ole):
    if not ole.exists('\x05DocumentSummaryInformation'): return None
    buf = ole.openstream('\x05DocumentSummaryInformation').read()
    if len(buf) < 28: return None
    for i in range(struct.unpack_from('<I', buf, 24)[0]):
        if buf[28 + i*20:44 + i*20] != USERDEF: continue
        off = struct.unpack_from('<I', buf, 44 + i*20)[0]
        size, count = struct.unpack_from('<II', buf, off)
        props, names = {}, {}
        for j in range(count):
            pid, poff = struct.unpack_from('<II', buf, off + 8 + j*8); props[pid] = off + poff
        if 0 in props:
            p = props[0]; ents = struct.unpack_from('<I', buf, p)[0]; p += 4
            for _ in range(ents):
                pid, cch = struct.unpack_from('<II', buf, p); p += 8
                names[buf[p:p+cch].split(b'\0')[0].decode('latin-1')] = pid; p += cch
        pid = names.get('_PID_HLINKS')
        if pid is None or pid not in props: return None
        q = props[pid]
        if struct.unpack_from('<I', buf, q)[0] != 65: return None
        nsize, ncount = struct.unpack_from('<II', buf, q + 4)
        return buf, q + 12, min(q + 12 + nsize, len(buf)), ncount
    return None

def entries(ole):
    got = pid_hlinks_blob(ole)
    if got is None: return None, 0
    buf, start, end, ncount = got
    if ncount % 6: return [], 0
    it = Item(buf, start, end)
    out = []
    for _ in range(ncount // 6):
        ok = True
        for _ in range(4):
            t = it.u32()
            if t != VT_I4: ok = False; break
            it.u32()
        if not ok: break
        target = it.string()
        if target is None: break
        sub = it.string()
        if sub is None: break
        out.append((target, sub))
    return out, ncount // 6

def valid_ids(path):
    ole = olefile.OleFileIO(str(path))
    buf = ole.openstream('PowerPoint Document').read()
    cur = current_user_edit(ole)
    built, declared = entries(ole)
    ole.close()
    offs, doc = directory(buf, cur)
    roots = census.live_roots(buf, offs, doc)
    atoms = census.link_ids(buf, roots)
    if built is None or not built:
        return set(atoms), len(atoms), declared, 'fallback'
    ids = set()
    for k in range(len(built)):
        ids.add(atoms[k] if k < len(atoms) else 0)
    return ids, len(built), declared, 'pid'

if __name__ == '__main__':
    for name in [l.strip() for l in open(sys.argv[1]) if l.strip()]:
        p = next(q for q in pathlib.Path('/home/user/sample-files').rglob('*') if q.name == name)
        ids, built, declared, how = valid_ids(p)
        print(f'{name[:58]:60s} declared {declared:3d} built {built:3d} via {how:9s} ids {sorted(ids)[:14]}')
