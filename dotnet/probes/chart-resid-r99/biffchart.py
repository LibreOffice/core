#!/usr/bin/env python3
"""Dump BIFF chart records of interest from an .xls (CHFRAMEPOS, CHCHART, CHPROPERTIES, CHAXESSET)."""
import struct, sys, olefile

IDS = {0x1002:'CHCHART',0x1014:'CHTYPEGROUP',0x1041:'CHAXESSET',0x104F:'CHFRAMEPOS',
       0x1044:'CHPROPERTIES',0x1033:'CHBEGIN',0x1034:'CHEND',0x1015:'CHLEGEND',
       0x1025:'CHTEXT',0x101D:'CHAXIS',0x1035:'CHPLOTFRAME',0x1032:'CHFRAME',
       0x0027:'CHPOS'}

def records(data):
    off = 0
    while off + 4 <= len(data):
        rid, rlen = struct.unpack_from('<HH', data, off)
        yield off, rid, data[off+4:off+4+rlen]
        off += 4 + rlen

def main(path):
    ole = olefile.OleFileIO(path)
    name = [n for n in ole.listdir() if n[-1] in ('Workbook','Book')][0]
    data = ole.openstream(name).read()
    depth = 0
    for off, rid, payload in records(data):
        if rid == 0x1034: depth -= 1
        nm = IDS.get(rid)
        if nm:
            extra = ''
            if rid == 0x104F and len(payload) >= 20:
                tl, br = struct.unpack_from('<HH', payload, 0)
                x, y, w, h = (struct.unpack_from('<h', payload, 4+i*4)[0] for i in range(4))
                extra = f' TL={tl} BR={br} rect=({x},{y},{w},{h})'
            if rid == 0x1002 and len(payload) >= 16:
                x, y, w, h = struct.unpack_from('<iiii', payload, 0)
                extra = f' pos=({x/65536:.2f},{y/65536:.2f}) size=({w/65536:.2f},{h/65536:.2f}) pt'
            if rid == 0x1044 and len(payload) >= 4:
                flags, empty = struct.unpack_from('<HH', payload, 0)
                extra = f' flags=0x{flags:04x} manplot={(flags>>2)&1} usemanplot={(flags>>3)&1}'
            if rid == 0x1041 and len(payload) >= 18:
                sid = struct.unpack_from('<H', payload, 0)[0]
                x, y, w, h = struct.unpack_from('<iiii', payload, 2)
                extra = f' id={sid} rect=({x},{y},{w},{h})'
            if rid == 0x1014 and len(payload) >= 20:
                x, y, w, h = struct.unpack_from('<hhhh', payload, 0)
                extra = f' rect=({x},{y},{w},{h})'
            if rid == 0x1015 and len(payload) >= 20:
                x, y, w, h = struct.unpack_from('<iiii', payload, 0)
                dock, spac, flags = struct.unpack_from('<BBH', payload, 16)
                extra = f' rect=({x},{y},{w},{h}) dock={dock} flags=0x{flags:04x}'
            print(f'{off:08x} {"  "*max(depth,0)}{nm} len={len(payload)}{extra}')
        if rid == 0x1033: depth += 1

main(sys.argv[1])
