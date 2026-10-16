#!/usr/bin/env python3
"""Which blips of a BIFF workbook are metafiles, and which shapes point at them.

Answers the question round 94 left on `TICAPCapability_Final.xls`: its eight paired
SRCAND blits are inside metafiles, but is any of those metafiles reachable from a
drawn shape at all?  A blip nobody's `pib` names is bytes in a store and no ink.

Walks the workbook stream's Escher records: the drawing group's blip store
(msofbtBSE 0xF007 inside msofbtBstoreContainer 0xF001) in order, then every
msofbtSpContainer's msofbtOPT for property 260 (`pib`), 267 (`pictureId`) and the
msofbtSp shape flags.  Prints one row per blip and one per shape.
"""
import struct, sys, zlib, pathlib
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from ole import read_ole
from esch import walk

SRCPAINT, SRCAND, SRCINVERT = 0x00EE0086, 0x008800C6, 0x00660046
WMF_BLITS = {0x0940, 0x0B41, 0x0F43, 0x0922, 0x0B23, 0x0D33}


def wmf_blits(d):
    if len(d) < 18: return
    pos, n = 18, 0
    while pos + 6 <= len(d) and n < 200000:
        size, fn = struct.unpack_from('<IH', d, pos)
        if size < 3: return
        if fn in WMF_BLITS:
            try:
                rop, = struct.unpack_from('<I', d, pos + 6)
                if fn in (0x0940, 0x0922):
                    ys, xs, dh, dw, yd, xd = struct.unpack_from('<6h', d, pos + 10)
                elif fn == 0x0F43:
                    usage, sh, sw, ys, xs, dh, dw, yd, xd = struct.unpack_from('<9h', d, pos + 10)
                elif fn == 0x0D33:
                    yield (fn, rop, None); pos += size * 2; n += 1; continue
                else:
                    sh, sw, ys, xs, dh, dw, yd, xd = struct.unpack_from('<8h', d, pos + 10)
                yield (fn, rop, (xd, yd, dw, dh))
            except struct.error:
                return
        pos += size * 2; n += 1


def blip_bytes(buf, body, ln, rt, inst):
    nuid = 2 if (inst & 1) else 1
    p = body + 16 * nuid
    if rt in (0xF01A, 0xF01B, 0xF01C):
        if p + 34 > len(buf): return b''
        comp = buf[p + 32]
        data = buf[p + 34: body + ln]
        if comp == 0:
            try: data = zlib.decompress(data)
            except Exception: return b''
        return data
    return buf[p + 1: body + ln]


def main(path):
    entries, read = read_ole(path)
    names = [e[0] for e in entries]
    stream = None
    for nm in ('Workbook', 'Book'):
        if nm in names:
            stream = read(nm); break
    if stream is None:
        print('no workbook stream'); return

    # Escher lives inside MSODRAWINGGROUP (0x00EB) and MSODRAWING (0x00EC) BIFF records.
    # Join each run of a record with its CONTINUE (0x003C) followers, as the file writes
    # the drawing group across many.
    pos = 0
    group = bytearray()
    persheet = []      # (index, bytes) one per MSODRAWING run
    cur = None
    while pos + 4 <= len(stream):
        rid, ln = struct.unpack_from('<HH', stream, pos)
        body = stream[pos + 4: pos + 4 + ln]
        if rid == 0x00EB:
            group += body; cur = ('g', None)
        elif rid == 0x00EC:
            persheet.append(bytearray(body)); cur = ('d', len(persheet) - 1)
        elif rid == 0x003C and cur is not None:
            if cur[0] == 'g': group += body
            else: persheet[cur[1]] += body
        elif rid not in (0x003C,):
            cur = None
        pos += 4 + ln

    print('# MSODRAWINGGROUP bytes: %d ; MSODRAWING runs: %d' % (len(group), len(persheet)))

    blips = []
    g = bytes(group)
    for depth, off, rt, inst, ver, ln, body in walk(g):
        if rt == 0xF007:      # msofbtBSE
            btype = g[body + 1]
            size, ref, fodelay = struct.unpack_from('<IIi', g, body + 20)
            inner = None
            if body + 36 + 8 <= body + ln:
                vi2, rt2, ln2 = struct.unpack_from('<HHI', g, body + 36)
                if rt2 in (0xF01A, 0xF01B, 0xF01C, 0xF01D, 0xF01E, 0xF01F, 0xF029, 0xF02A):
                    inner = (rt2, vi2 >> 4, ln2, body + 44)
            blips.append((len(blips) + 1, btype, size, ref, fodelay, inner))

    print('idx\tbtype\tsize\trefs\tfoDelay\trecord\tbytes\tlone\tpaired\tnblits')
    for idx, btype, size, ref, fodelay, inner in blips:
        if inner is None:
            print('%d\t%d\t%d\t%d\t%d\t-\t-\t-\t-\t-' % (idx, btype, size, ref, fodelay))
            continue
        rt2, inst2, ln2, dstart = inner
        data = blip_bytes(g, dstart - 8, ln2, rt2, inst2)
        lone = paired = 0
        b = list(wmf_blits(data)) if rt2 == 0xF01B else []
        i = 0
        while i < len(b):
            fn, rop, rect = b[i]
            if rop == SRCAND:
                if i + 1 < len(b) and b[i+1][1] in (SRCPAINT, SRCINVERT) and b[i+1][2] == rect:
                    paired += 1; i += 2; continue
                if i and b[i-1][1] == SRCPAINT and b[i-1][2] == rect:
                    i += 1; continue
                lone += 1
            elif rop == SRCPAINT and i + 1 < len(b) and b[i+1][1] == SRCAND and b[i+1][2] == rect:
                paired += 1; i += 2; continue
            i += 1
        print('%d\t%d\t%d\t%d\t%d\t0x%04X\t%d\t%d\t%d\t%d'
              % (idx, btype, size, ref, fodelay, rt2, len(data), lone, paired, len(b)))

    # every shape's pib
    print()
    print('run\tspid\tshapetype\tflags\tpib\tpictureId\tfillBlip')
    for ri, d in enumerate(persheet):
        d = bytes(d)
        cur_sp = None
        for depth, off, rt, inst, ver, ln, body in walk(d):
            if rt == 0xF00A:      # msofbtSp
                spid, flags = struct.unpack_from('<II', d, body)
                cur_sp = (spid, inst, flags)
            elif rt == 0xF00B and cur_sp:   # msofbtOPT
                p = body; pib = pid = fb = 0
                nprops = inst
                vals = {}
                for k in range(nprops):
                    if p + 6 > body + ln: break
                    pidf, val = struct.unpack_from('<HI', d, p)
                    vals[pidf & 0x3FFF] = val
                    p += 6
                print('%d\t%d\t%d\t0x%X\t%d\t%d\t%d' % (
                    ri, cur_sp[0], cur_sp[1], cur_sp[2],
                    vals.get(260, 0), vals.get(267, 0), vals.get(390, 0)))
                cur_sp = None


if __name__ == '__main__':
    main(sys.argv[1])
