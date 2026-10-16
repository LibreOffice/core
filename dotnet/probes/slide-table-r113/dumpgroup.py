#!/usr/bin/env python3
"""Dump every member of every `.ppt` table group: shape type, anchor, and line width.

The anchors are master units, 1/576 inch, so a point is eight of them; the widths
are EMU.  This is the input side of both halves of round 113: the row grid the
reference re-derives is seeded from the non-line members' tops and measured to the
group's own bottom, and each line member's `DFF_Prop_lineWidth` (459) is what
`ApplyCellLineAttributes` turns into a cell border.

Usage: dumpgroup.py <file.ppt>
"""
import struct, sys, pathlib
import olefile
TABLE_PROPS, TABLE_ROWS = 927, 928

def records(data, off, end, depth=0):
    while off + 8 <= end:
        ver, typ, ln = struct.unpack_from('<HHI', data, off)
        body, bend = off + 8, off + 8 + ln
        if bend > end: return
        yield typ, ver >> 4, body, ln, depth
        if (ver & 0xF) == 0xF:
            yield from records(data, body, bend, depth + 1)
        off = bend

def props(data, body, count, length):
    out = {}
    for i in range(min(count, length // 6)):
        raw, val = struct.unpack_from('<HI', data, body + i * 6)
        out[raw & 0x3FFF] = val
    return out

def sp_info(data, off, end):
    shtype = -1; p = {}; anchor = None
    for typ, inst, body, ln, _ in records(data, off, end):
        if typ == 0xF00A and ln >= 8: shtype = inst
        elif typ == 0xF00B and not p: p = props(data, body, inst, ln)
        elif typ == 0xF010 and ln >= 16:
            anchor = struct.unpack_from('<iiii', data, body)
        elif typ == 0xF00F and ln >= 16:
            anchor = struct.unpack_from('<iiii', data, body)
    return shtype, p, anchor

def group_is_table(data, off, end):
    while off + 8 <= end:
        ver, typ, ln = struct.unpack_from('<HHI', data, off)
        body, bend = off + 8, off + 8 + ln
        if bend > end: return False
        if typ == 0xF004:
            for t2, i2, b2, l2, _ in records(data, body, bend):
                if t2 == 0xF122:
                    p = props(data, b2, i2, l2)
                    return bool(p.get(TABLE_PROPS, 0) & 3) and TABLE_ROWS in p
            return False
        off = bend
    return False

path = pathlib.Path(sys.argv[1])
with olefile.OleFileIO(str(path)) as f:
    data = f.openstream('PowerPoint Document').read()

def walk(off, end, in_table, gid):
    while off + 8 <= end:
        ver, typ, ln = struct.unpack_from('<HHI', data, off)
        body, bend = off + 8, off + 8 + ln
        if bend > end: return
        if typ == 0xF003:
            t = group_is_table(data, body, bend)
            if t: print(f'--- table group at {off} ---')
            walk(body, bend, t, off if t else gid)
        elif typ == 0xF004 and in_table:
            shtype, p, anchor = sp_info(data, body, bend)
            print(f'  type {shtype:4} anchor {anchor} lw {p.get(459)}')
        elif (ver & 0xF) == 0xF:
            walk(body, bend, False, gid)
        off = bend
walk(0, len(data), False, 0)
