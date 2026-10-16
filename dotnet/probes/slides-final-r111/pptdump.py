#!/usr/bin/env python3
"""Dump PPT records relevant to the picture bullet: ExtendedBuGraAtom and
ExtendedParagraphMasterAtom, plus per-shape ExtendedParagraphAtom."""
import sys, struct, olefile

def recs(buf, off, end, depth=0):
    while off + 8 <= end:
        ver_inst, rtype, rlen = struct.unpack_from('<HHI', buf, off)
        ver = ver_inst & 0xF
        inst = ver_inst >> 4
        body = off + 8
        stop = min(body + rlen, end)
        yield (depth, off, ver, inst, rtype, rlen, body, stop)
        if ver == 0xF:
            yield from recs(buf, body, stop, depth + 1)
        off = body + rlen

NAMES = {2000:'List',2040:'ExtendedBuGraContainer',2041:'ExtendedBuGraAtom',
         4012:'ExtendedParagraphAtom',4013:'ExtendedParagraphMasterAtom',
         5000:'ProgTags',5002:'ProgBinaryTag',4026:'CString',5003:'BinaryTagData',
         1000:'Document',1016:'List?',1035:'PPDrawing',1036:'MainMaster',1006:'Slide'}

def main(path):
    ole = olefile.OleFileIO(path)
    buf = ole.openstream('PowerPoint Document').read()
    print('stream', len(buf))
    for d, off, ver, inst, rtype, rlen, body, stop in recs(buf, 0, len(buf)):
        if rtype in (2040, 2041, 4013, 4012, 2000):
            print(f'{"  "*d}@{off} type={rtype}({NAMES.get(rtype,"")}) ver={ver} inst={inst} len={rlen}')
            if rtype == 4013:
                depth = struct.unpack_from('<H', buf, body)[0]
                print(f'{"  "*d}  masterAtom instance={inst} depth={depth}')
                p = body + 2
                for i in range(min(depth, 5)):
                    if p + 4 > stop: break
                    mask = struct.unpack_from('<I', buf, p)[0]; p += 4
                    blip = anm = None; scheme = None
                    if mask & 0x00800000: blip = struct.unpack_from('<H', buf, p)[0]; p += 2
                    if mask & 0x02000000: anm = struct.unpack_from('<H', buf, p)[0]; p += 2
                    if mask & 0x01000000: scheme = struct.unpack_from('<I', buf, p)[0]; p += 4
                    if mask & 0x04000000: p += 4
                    cmask = struct.unpack_from('<I', buf, p)[0]; p += 4
                    if cmask & 0x100000: p += 4
                    print(f'{"  "*d}   lvl{i} mask=0x{mask:08x} blip={blip} hasAnm={anm} scheme={scheme} cmask=0x{cmask:08x}')
            if rtype == 2041:
                ntype = struct.unpack_from('<H', buf, body)[0]
                print(f'{"  "*d}  buGra instance={inst} type={ntype} blipbytes={rlen-2}')

main(sys.argv[1])
