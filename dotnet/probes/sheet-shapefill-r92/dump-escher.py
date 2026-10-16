#!/usr/bin/env python3
"""Dump every Escher shape's msofbtOPT property table out of a BIFF8 workbook.

Walks the Workbook stream, joins the MSODRAWING/CONTINUE payloads per sheet substream
exactly as XclImpDrawing::ReadMsoDrawing does, and prints one line per property of every
shape container, with the shape name (property 896) where it is stated.

Usage: dump-escher.py <file.xls> [sheet-index]
"""
import struct
import sys

import olefile

MSODRAWING = 0x00EC
MSODRAWINGGROUP = 0x00EB
CONTINUE = 0x003C
BOF = 0x0809
EOF = 0x000A
BOUNDSHEET = 0x0085
OBJ = 0x005D
TXO = 0x01B6

# The property names this round cares about; everything else prints by number.
NAMES = {
    127: "fLockAgainstGrouping/protection",
    128: "txdir", 129: "?", 133: "?",
    134: "anchorText", 135: "txflTextFlow",
    260: "pib",
    261: "pibName",
    262: "pibFlags",
    263: "pictureTransparent",
    319: "geoRight",
    320: "geoBottom",
    321: "shapePath",
    324: "fFillOK/geometry-bools",
    325: "?",
    327: "adjust",
    343: "geometry-bools",
    384: "fillType",
    385: "fillColor",
    386: "fillOpacity",
    387: "fillBackColor",
    388: "fillBackOpacity",
    389: "fillCrMod",
    390: "fillBlip",
    401: "fillRectRight",
    402: "fillRectBottom",
    443: "fill-bools (fFilled bit0)",
    448: "lineColor",
    449: "lineOpacity",
    450: "lineBackColor",
    459: "lineWidth",
    460: "lineMiterLimit",
    461: "lineStyle",
    462: "lineDashing",
    511: "line-bools (fLine bit3)",
    512: "shadowType",
    513: "shadowColor",
    519: "shadowOpacity",
    575: "shadow-bools",
    896: "wzName",
    959: "group-bools (fHidden bit1, fPrint bit2)",
}


def records(data):
    at = 0
    while at + 4 <= len(data):
        rid, size = struct.unpack_from("<HH", data, at)
        at += 4
        yield rid, data[at:at + size]
        at += size


def walk_escher(buf, out, depth=0):
    at = 0
    while at + 8 <= len(buf):
        ver_inst, rtype, size = struct.unpack_from("<HHI", buf, at)
        ver = ver_inst & 0x0F
        inst = ver_inst >> 4
        body = buf[at + 8:at + 8 + size]
        if ver == 0x0F:
            walk_escher(body, out, depth + 1)
        else:
            out.append((rtype, inst, body))
        at += 8 + size


def walk_shapes(buf, shapes, depth=0):
    """Collect (spContainer-index, [records]) for each msofbtSpContainer."""
    at = 0
    while at + 8 <= len(buf):
        ver_inst, rtype, size = struct.unpack_from("<HHI", buf, at)
        ver = ver_inst & 0x0F
        body = buf[at + 8:at + 8 + size]
        if rtype == 0xF004:  # msofbtSpContainer
            recs = []
            walk_escher(body, recs)
            shapes.append(recs)
            # a container may nest (group), keep walking inside too
            walk_shapes(body, shapes, depth + 1)
        elif ver == 0x0F:
            walk_shapes(body, shapes, depth + 1)
        at += 8 + size


def properties(body, inst):
    """msofbtOPT: inst properties of six bytes, then the complex data."""
    props = []
    n = inst
    at = 0
    complex_at = 6 * n
    for _ in range(n):
        if at + 6 > len(body):
            break
        pid_raw, value = struct.unpack_from("<HI", body, at)
        at += 6
        pid = pid_raw & 0x3FFF
        is_blip = bool(pid_raw & 0x4000)
        is_complex = bool(pid_raw & 0x8000)
        extra = b""
        if is_complex:
            extra = body[complex_at:complex_at + value]
            complex_at += value
        props.append((pid, value, is_blip, is_complex, extra))
    return props


def main():
    path = sys.argv[1]
    want = sys.argv[2] if len(sys.argv) > 2 else None
    ole = olefile.OleFileIO(path)
    name = "Workbook" if ole.exists("Workbook") else "Book"
    data = ole.openstream(name).read()

    # split into substreams on BOF
    subs = []
    cur = None
    for rid, body in records(data):
        if rid == BOF:
            cur = []
            subs.append(cur)
        if cur is not None:
            cur.append((rid, body))

    sheetnames = []
    for rid, body in subs[0]:
        if rid == BOUNDSHEET:
            ln = body[6]
            flags = body[7]
            nm = body[8:8 + ln * (2 if flags & 1 else 1)]
            sheetnames.append(nm.decode("utf-16-le" if flags & 1 else "latin-1"))

    print(f"# {path}: {len(subs)} substreams, sheets {sheetnames}")

    for i, sub in enumerate(subs):
        dff = bytearray()
        last = None
        for rid, body in sub:
            if rid in (MSODRAWING, MSODRAWINGGROUP):
                dff += body
                last = rid
            elif rid == CONTINUE and last in (MSODRAWING, MSODRAWINGGROUP):
                dff += body
            elif rid not in (CONTINUE,):
                last = None
        if not dff:
            continue
        label = sheetnames[i - 1] if 0 < i <= len(sheetnames) else f"sub{i}"
        if want is not None and want.lower() not in label.lower():
            continue
        shapes = []
        walk_shapes(bytes(dff), shapes)
        print(f"\n=== substream {i} ({label}): {len(dff)} dff bytes, {len(shapes)} shapes")
        for si, recs in enumerate(shapes):
            opt = [(inst, body) for (rt, inst, body) in recs if rt == 0xF00B]
            sp = [(inst, body) for (rt, inst, body) in recs if rt == 0xF00A]
            anchor = [b for (rt, _, b) in recs if rt == 0xF010]
            shape_type = sp[0][0] if sp else None
            flags = struct.unpack_from("<I", sp[0][1], 4)[0] if sp else 0
            nm = ""
            allprops = []
            for inst, body in opt:
                allprops += properties(body, inst)
            for pid, value, is_blip, is_complex, extra in allprops:
                if pid == 896 and is_complex:
                    nm = extra.decode("utf-16-le", "replace").rstrip("\x00")
            print(f"-- shape {si}: spType={shape_type} spFlags=0x{flags:08x} "
                  f"name={nm!r} anchor={'yes' if anchor else 'no'}")
            for pid, value, is_blip, is_complex, extra in allprops:
                label2 = NAMES.get(pid, "")
                print(f"     {pid:4d} 0x{value:08x} {'C' if is_complex else ' '}"
                      f"{'B' if is_blip else ' '} {label2}")


if __name__ == "__main__":
    main()
