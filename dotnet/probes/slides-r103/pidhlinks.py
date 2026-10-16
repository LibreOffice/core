#!/usr/bin/env python3
"""How many hyperlinks 26.2.4.2 believes a .ppt has, and which ExHyperlink records get an id.

`SdrPowerPointImport::Import` builds `m_aHyperList` from the **`_PID_HLINKS` blob of
`\\005DocumentSummaryInformation`** -- six OLE properties per hyperlink, and the loop breaks
on the first one whose type is not VT_I4 or whose string will not read
(`sd/source/filter/ppt/pptin.cxx`:353-518, read in this checkout, which declares
27.2.0.0.alpha0+).  Only *then* does it walk the `ExObjList`, giving the i-th entry of that
list the id of the i-th `ExHyperlink` record in stream order and **stopping when the list runs
out** (`:530-547`).

So the number of hyperlinks a .ppt has, for the reference, is the property blob's -- not the
ExObjList's.  A text range whose `InteractiveInfoAtom.exHyperlinkId` is not one of the ids that
were handed out finds nothing in `m_aHyperList` and is **not made a field**
(`filter/source/msfilter/svdfppt.cxx`:6907-6941).

    pidhlinks.py <file.ppt>...

prints, per document: the blob's declared count, the count the loop actually reads, the ids the
ExObjList walk assigns, every `ExHyperlink`/`ExHyperlinkAtom` id in the stream, and every
`InteractiveInfoAtom.exHyperlinkId` a text range names.
"""
import struct, sys
import olefile

VT_I4, VT_LPSTR, VT_LPWSTR, VT_BLOB = 3, 30, 31, 65
USERDEF_FMTID = b'\x05\xd5\xcd\xd5\x9c\x2e\x1b\x10\x93\x97\x08\x00\x2b\x2c\xf9\xae'


def sections(data):
    """(fmtid, offset) of every section of an OLE property set stream."""
    count = struct.unpack_from('<I', data, 24)[0]
    out = []
    for i in range(count):
        fmtid = data[28 + i * 20: 28 + i * 20 + 16]
        off = struct.unpack_from('<I', data, 28 + i * 20 + 16)[0]
        out.append((fmtid, off))
    return out


def properties(data, off):
    """{propid: absolute offset} for one section."""
    count = struct.unpack_from('<I', data, off + 4)[0]
    out = {}
    for i in range(count):
        pid, poff = struct.unpack_from('<II', data, off + 8 + i * 8)
        out[pid] = off + poff
    return out


def dictionary(data, off):
    """propid -> name, from the section's property 0."""
    n = struct.unpack_from('<I', data, off)[0]
    p = off + 4
    out = {}
    for _ in range(n):
        pid, ln = struct.unpack_from('<II', data, p)
        raw = data[p + 8: p + 8 + ln]
        out[pid] = raw.split(b'\0')[0].decode('latin1')
        p += 8 + ln
    return out


def read_str(data, p, ucs2):
    """`PropItem::Read` (sd/source/filter/ppt/propread.cxx:73-150): a length in BYTES, the
    bytes themselves, then a dword alignment.  A VT_LPSTR under a UCS-2 code page is read as
    wide characters, which is what a PowerPoint blob writes."""
    (ln,) = struct.unpack_from('<I', data, p)
    p += 4
    raw = data[p:p + ln]
    p += ln + ((4 - (ln & 3)) & 3)
    if ucs2:
        return raw.decode('utf-16-le', 'replace').split('\0')[0], p
    return raw.split(b'\0')[0].decode('latin1'), p


def read_prop_string(blob, p):
    """`PropItem::Read` (propread.cxx:73-186): a type, a count, the characters, a dword
    alignment -- and FALSE, which stops the caller, when the last character it can reach is
    not a terminator.  A truncated buffer therefore ends the hyperlink list rather than
    shortening one string."""
    if p + 8 > len(blob):
        return None
    vt, n = struct.unpack_from('<II', blob, p)
    start = p + 8
    if vt == VT_LPWSTR:
        if n == 0 or start + n * 2 > len(blob):
            return None
        if blob[start + (n - 1) * 2: start + (n - 1) * 2 + 2] != b'\0\0':
            return None
        return start + n * 2 + (2 if n & 1 else 0)
    if vt == VT_LPSTR:
        if n == 0 or start + n > len(blob):
            return None
        if blob[start + n - 1] != 0:
            return None
        return start + n + ((4 - (n & 3)) & 3)
    return None


def hyperlinks(blob):
    """Walk the (already clamped) property buffer exactly as pptin.cxx:392-518 does."""
    if len(blob) < 12:
        return None, 0
    (vt,) = struct.unpack_from('<I', blob, 0)
    if vt != VT_BLOB:
        return None, 0
    _size, nprops = struct.unpack_from('<II', blob, 4)
    if nprops % 6:
        return nprops, 0
    declared = nprops // 6
    p = 12
    read = 0
    for _ in range(declared):
        typed = True
        for _field in range(4):
            if p + 8 > len(blob) or struct.unpack_from('<I', blob, p)[0] != VT_I4:
                typed = False
                break
            p += 8
        if not typed:
            break
        p = read_prop_string(blob, p)
        if p is None:
            break
        p = read_prop_string(blob, p)
        if p is None:
            break
        read += 1
    return declared, read


def records(stream):
    """Every (type, instance, offset, length) in a PPT record stream, walked as a tree."""
    out = []

    def walk(start, end):
        p = start
        while p + 8 <= end:
            verinst, rtype, rlen = struct.unpack_from('<HHI', stream, p)
            out.append((rtype, verinst >> 4, p, rlen))
            body = p + 8
            if (verinst & 0x0F) == 0x0F:
                walk(body, min(body + rlen, end))
            p = body + rlen

    walk(0, len(stream))
    return out


def main(paths):
    for path in paths:
        ole = olefile.OleFileIO(path)
        name = path.rsplit('/', 1)[-1]
        declared = read = None
        if ole.exists('\x05DocumentSummaryInformation'):
            data = ole.openstream('\x05DocumentSummaryInformation').read()
            for fmtid, off in sections(data):
                if fmtid != USERDEF_FMTID:
                    continue
                props = properties(data, off)
                if 0 not in props:
                    continue
                (secsize,) = struct.unpack_from('<I', data, off)
                names = dictionary(data, props[0])
                for pid, nm in names.items():
                    if nm == '_PID_HLINKS' and pid in props:
                        # Section::Read, propread.cxx:443-447 -- a length less a POSITION.
                        clamp = secsize - off
                        end = len(data)
                        if 0 <= clamp < end - props[pid]:
                            end = props[pid] + clamp
                        clamped = end - props[pid]
                        declared, read = hyperlinks(data[props[pid]:end])
        ids, ranges = [], []
        if ole.exists('PowerPoint Document'):
            stream = ole.openstream('PowerPoint Document').read()
            for rtype, _inst, off, rlen in records(stream):
                if rtype == 4051 and rlen >= 4:             # ExHyperlinkAtom
                    ids.append(struct.unpack_from('<I', stream, off + 8)[0])
                if rtype == 4083 and rlen >= 8:             # InteractiveInfoAtom
                    ranges.append(struct.unpack_from('<I', stream, off + 8 + 4)[0])
        ole.close()
        assigned = ids[:read] if read else []
        print(f'{name}\tblob_declared={declared}\tblob_read={read}\t'
              f'exhyperlink_atoms={len(ids)}\tids={ids}\tassigned={assigned}\t'
              f'ranges={sorted(set(ranges))}\tresolvable='
              f'{sorted(set(ranges) & set(assigned))}')


if __name__ == '__main__':
    main(sys.argv[1:])
