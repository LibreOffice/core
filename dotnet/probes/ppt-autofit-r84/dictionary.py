#!/usr/bin/env python3
"""Does `Section::GetDictionary` find `_PID_HLINKS` at all?

`sd/source/filter/ppt/propread.cxx`:262-309 reads the user-defined section's dictionary
(property id 0) as a bare sequence of `id, size, name` triples and **applies no padding**
between entries, while MS-OLEPS pads every entry of a *Unicode* (code page 1200) dictionary
to a four-byte boundary.  It also stops at the first empty name (`:305-306`).  So on a
Unicode dictionary whose first name has an odd character count, every entry after it is
read from the wrong offset and the map never gains `_PID_HLINKS` -- and with `aDict` empty
`ImplSdPPTImport::Import` (`pptin.cxx`:361-364) never builds `m_aHyperList`, so no text
range in the whole document becomes a field.
"""
import struct, sys, pathlib
import olefile

USERDEF = bytes([0x05,0xd5,0xcd,0xd5,0x9c,0x2e,0x1b,0x10,0x93,0x97,0x08,0x00,0x2b,0x2c,0xf9,0xae])
CODEPAGE_PID = 1

def section(buf, fmtid):
    if len(buf) < 28: return None
    for i in range(struct.unpack_from('<I', buf, 24)[0]):
        if buf[28 + i*20:44 + i*20] == fmtid:
            return struct.unpack_from('<I', buf, 44 + i*20)[0]
    return None

def properties(buf, off):
    size, count = struct.unpack_from('<II', buf, off)
    out = {}
    for j in range(count):
        pid, poff = struct.unpack_from('<II', buf, off + 8 + j*8)
        out[pid] = off + poff
    return out

def dictionary(buf, off, unicode_names):
    """LibreOffice's own reading: no padding, stop at the first empty name."""
    p = off
    n = struct.unpack_from('<I', buf, p)[0]; p += 4
    names = {}
    for _ in range(n):
        if p + 8 > len(buf): break
        pid, size = struct.unpack_from('<II', buf, p); p += 8
        if size > len(buf) - p: break
        if unicode_names: size >>= 1
        if not size: continue
        if unicode_names:
            raw = buf[p:p + 2*size]; p += 2*size
            s = raw.decode('utf-16-le', 'replace')
        else:
            raw = buf[p:p + size]; p += size
            s = raw.decode('latin-1')
        s = s.split('\0')[0]
        if not s: break
        names[s] = pid
    return names

def probe(path):
    ole = olefile.OleFileIO(str(path))
    if not ole.exists('\x05DocumentSummaryInformation'):
        ole.close(); return ('no-stream', None, {})
    buf = ole.openstream('\x05DocumentSummaryInformation').read()
    ole.close()
    off = section(buf, USERDEF)
    if off is None: return ('no-section', None, {})
    props = properties(buf, off)
    codepage = None
    if CODEPAGE_PID in props:
        t, v = struct.unpack_from('<Ih', buf, props[CODEPAGE_PID])
        codepage = v & 0xFFFF
    if 0 not in props: return ('no-dictionary', codepage, {})
    names = dictionary(buf, props[0], codepage == 1200)
    return ('ok', codepage, names)

if __name__ == '__main__':
    for name in [l.strip() for l in open(sys.argv[1]) if l.strip()]:
        p = next(q for q in pathlib.Path('/home/user/sample-files').rglob('*') if q.name == name)
        state, cp, names = probe(p)
        print(f'{name[:56]:58s} {state:14s} codepage {str(cp):6s} '
              f'_PID_HLINKS {"yes" if "_PID_HLINKS" in names else "NO ":3s} names {list(names)[:4]}')
