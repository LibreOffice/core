#!/usr/bin/env python3
"""Every live slide's Escher shapes: type, client anchor, and a chosen set of OPT properties."""
import struct, sys, pathlib
import olefile
from persist import directory, current_user_edit, header

SLIDE, PPDRAWING = 1006, 1036
SPCONT, SP, OPT, CLIENTANCHOR, CHILDANCHOR, CLIENTTEXTBOX, OPT2, OPT3 = (
    0xF004, 0xF00A, 0xF00B, 0xF010, 0xF00F, 0xF00D, 0xF121, 0xF122)

def records(buf, s, e):
    p = s
    while p + 8 <= e:
        vi, rt, rl = struct.unpack_from('<HHI', buf, p)
        body, stop = p + 8, p + 8 + rl
        if stop > e: return
        yield (vi & 0x0F, vi >> 4, rt, body, stop)
        p = stop

def walk(buf, s, e):
    for ver, inst, rt, b, st in records(buf, s, e):
        yield (ver, inst, rt, b, st)
        if ver == 0x0F:
            yield from walk(buf, b, st)

def props(buf, s, e):
    """OPT: instance = count of six-byte entries, then the complex payloads."""
    out = {}
    n = (e - s) // 6
    comp = s + n * 6
    for i in range(n):
        pid, val = struct.unpack_from('<HI', buf, s + i * 6)
        cid, bid, complx = pid & 0x3FFF, (pid >> 14) & 1, (pid >> 15) & 1
        if complx:
            out[cid] = ('C', buf[comp:comp + val]); comp += val
        else:
            out[cid] = ('V', val)
    return out

def live_roots(buf, offsets, doc):
    seen, roots = set(), []
    for pid in [doc] + [k for k in sorted(offsets) if k != doc]:
        off = offsets.get(pid)
        if off is None or off in seen: continue
        seen.add(off)
        h = header(buf, off)
        if h: roots.append((h[1], off, h[3]))
    return roots

def read(path):
    ole = olefile.OleFileIO(str(path))
    buf = ole.openstream('PowerPoint Document').read()
    cur = current_user_edit(ole)
    ole.close()
    offsets, doc = directory(buf, cur)
    return buf, live_roots(buf, offsets, doc)

def anchor(buf, b, e):
    n = e - b
    if n >= 16:
        l, t, r, bo = struct.unpack_from('<iiii', buf, b)
        return (l, t, r, bo)
    if n >= 8:
        t, l, r, bo = struct.unpack_from('<hhhh', buf, b)
        return (l, t, r, bo)
    return None

def shapes_of(buf, s, e):
    for ver, inst, rt, b, st in records(buf, s, e):
        if rt == SPCONT:
            info = dict(type=None, flags=0, anchor=None, props={}, text=False)
            for v2, i2, t2, b2, s2 in records(buf, b, st):
                if t2 == SP:
                    info['type'] = i2
                    info['flags'] = struct.unpack_from('<I', buf, b2 + 4)[0] if s2 - b2 >= 8 else 0
                elif t2 == OPT:
                    info['props'].update(props(buf, b2, s2))
                elif t2 in (OPT2, OPT3):
                    info['props'].update(props(buf, b2, s2))
                elif t2 in (CLIENTANCHOR, CHILDANCHOR):
                    info['anchor'] = anchor(buf, b2, s2)
                elif t2 == CLIENTTEXTBOX:
                    info['text'] = True
            yield info
        elif ver == 0x0F:
            yield from shapes_of(buf, b, st)
