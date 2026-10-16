#!/usr/bin/env python3
"""Does the document's user-defined property section carry _PID_HLINKS, and how many links?

`pptin.cxx`:353-518 fills `m_aHyperList` from the `_PID_HLINKS` VT_BLOB of the user-defined
section of `\x05DocumentSummaryInformation`, six properties per link.  Only then does
:531-537 assign each entry an index from the `ExObjList`'s `ExHyperlinkAtom`s, in order --
so with no `ExObjList` (or fewer `ExHyperlink` records than entries) the remaining entries
keep the `nIndex = 0` they were constructed with, and an `InteractiveInfoAtom` stating
`exHyperlinkId = 0` matches them.
"""
import struct, sys, pathlib, olefile

USERDEF_FMTID = bytes([0x05,0xd5,0xcd,0xd5,0x9c,0x2e,0x1b,0x10,0x93,0x97,0x08,0x00,0x2b,0x2c,0xf9,0xae])
VT_BLOB, VT_I4 = 65, 3

def sections(buf):
    if len(buf) < 28: return
    n = struct.unpack_from('<I', buf, 24)[0]
    for i in range(n):
        fmtid = buf[28 + i*20 : 44 + i*20]
        off = struct.unpack_from('<I', buf, 44 + i*20)[0]
        yield fmtid, off

def pid_hlinks(buf):
    for fmtid, off in sections(buf):
        if fmtid != USERDEF_FMTID: continue
        size, count = struct.unpack_from('<II', buf, off)
        props = {}
        for i in range(count):
            pid, poff = struct.unpack_from('<II', buf, off + 8 + i*8)
            props[pid] = off + poff
        # PID 0 is the dictionary: entry count, then (pid, cch, name) triples
        name_of = {}
        if 0 in props:
            p = props[0]
            entries = struct.unpack_from('<I', buf, p)[0]
            p += 4
            for _ in range(entries):
                pid, cch = struct.unpack_from('<II', buf, p)
                p += 8
                name = buf[p:p+cch].split(b'\0')[0].decode('latin-1')
                p += cch
                name_of[name] = pid
        pid = name_of.get('_PID_HLINKS')
        if pid is None or pid not in props: return None
        p = props[pid]
        vt = struct.unpack_from('<I', buf, p)[0]
        if vt != VT_BLOB: return ('not-blob', vt)
        nsize, ncount = struct.unpack_from('<II', buf, p + 4)
        return ('blob', ncount, ncount // 6 if ncount % 6 == 0 else None)
    return None

for name in sys.argv[1:]:
    p = next(q for q in pathlib.Path('/home/user/sample-files').rglob('*') if q.name == name)
    ole = olefile.OleFileIO(str(p))
    if ole.exists('\x05DocumentSummaryInformation'):
        buf = ole.openstream('\x05DocumentSummaryInformation').read()
        print(name, '->', pid_hlinks(buf))
    else:
        print(name, '-> no DocumentSummaryInformation')
    ole.close()
