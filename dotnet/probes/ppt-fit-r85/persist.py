#!/usr/bin/env python3
"""The PowerPoint persist directory, ported from `PptPersistDirectory.Read`.

A `.ppt` stream holds every superseded version of every object, and nothing in the record
tree says which is current.  A census that walks the stream top to bottom therefore counts
orphans -- which is how this round's first census reported 235 text-range hyperlink atoms
in 23 documents when the live tree holds far fewer.
"""
import struct

CURRENTUSERATOM, USEREDITATOM, PERSISTPTRBLOCK = 0x0FF6, 0x0FF5, 0x1772

def header(buf, off):
    if off < 0 or off + 8 > len(buf): return None
    vi, rt, rl = struct.unpack_from('<HHI', buf, off)
    if off + 8 + rl > len(buf): rl = len(buf) - off - 8
    return (vi & 0x0F, rt, off + 8, off + 8 + rl)

def user_edit(buf, off):
    h = header(buf, off)
    if not h or h[1] != USEREDITATOM or h[3] - h[2] < 24: return None
    c = h[2]
    prev, pdir, docref, maxw = struct.unpack_from('<IIII', buf, c + 8)
    return dict(prev=prev, pdir=pdir, doc=docref, maxw=maxw, end=h[3])

def current_user_edit(ole):
    if not ole.exists('Current User'): return 0
    b = ole.openstream('Current User').read()
    h = header(b, 0)
    if not h or h[1] != CURRENTUSERATOM or h[3] - h[2] < 12: return 0
    return struct.unpack_from('<I', b, h[2] + 8)[0]

def directory(buf, current):
    edit = user_edit(buf, current)
    if edit is None:
        last = None
        p = 0
        while p + 8 <= len(buf):
            h = header(buf, p)
            if not h: break
            if h[1] == USEREDITATOM: last = user_edit(buf, p)
            p = h[3]
        edit = last
    if edit is None: return {}, 0
    doc = edit['doc']
    offsets, pos, guard = {}, edit['end'], 0
    while pos > 0 and guard < 100000:
        guard += 1
        h = header(buf, edit['pdir'])
        if h and h[1] == PERSISTPTRBLOCK:
            c, e = h[2], h[3]
            limit = edit['maxw'] + 1
            while c + 4 <= e:
                packed = struct.unpack_from('<I', buf, c)[0]; c += 4
                first, count = packed & 0x000FFFFF, packed >> 20
                for i in range(count):
                    if c + 4 > e: break
                    off = struct.unpack_from('<I', buf, c)[0]; c += 4
                    if first + i > limit or off >= len(buf): continue
                    offsets.setdefault(first + i, off)
        if edit['prev'] == 0 or edit['prev'] >= pos: break
        nxt = user_edit(buf, edit['prev'])
        if nxt is None: break
        pos, edit = edit['prev'], nxt
    return offsets, doc
