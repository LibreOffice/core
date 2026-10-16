#!/usr/bin/env python3
"""Dump every inline picture in a .doc: its PICF, its FSP, its OPT, and its BLIP kind.

    inline-pics.py <file.doc> [more.doc ...]

Walks records, never regex. The Data stream is scanned for OfficeArtSpContainer headers
exactly as crop-wiring-01's census does (version nibble 0x0F, length fits, first child an
FSP of exactly 8 bytes), because an inline picture's container hangs off a sprmCPicLocation
offset that only the piece table knows.

For each container found it then walks *backwards* to the PICF that must precede it: the
PICF's cbHeader is the distance from the start of the PICF to the start of the container,
so a candidate offset is confirmed when the two agree and the metafile-type field is one
the Escher path uses.

Reported per picture:
  PICF     lcb cbHeader mm xExt yExt dxaGoal dyaGoal mx my dxaCrop*
  FSP      spid, flags  (0x10 = OLEShape)
  OPT      pib(260) pibFlags(262) cropFromTop/Bottom/Left/Right(256..259)
  BLIP     the record type that follows the container inside the inline container
  extras   whether the container carries an OLE record, a group, or client data
"""
import struct
import sys
from pathlib import Path

import olefile

SP_CONTAINER = 0xF004
SP = 0xF00A
OPT = 0xF00B
OPT2 = 0xF121
OPT3 = 0xF122
BSTORE = 0xF001
BSE = 0xF007

BLIP_NAMES = {
    0xF01A: "EMF", 0xF01B: "WMF", 0xF01C: "PICT", 0xF01D: "JPEG",
    0xF01E: "PNG", 0xF01F: "DIB", 0xF029: "TIFF",
}
BLIP_TYPE_BY_WIN = {2: "EMF", 3: "WMF", 4: "PICT", 5: "JPEG", 6: "PNG", 7: "DIB", 17: "TIFF"}

SHAPE_FLAGS = [
    (0x0001, "Group"), (0x0002, "Child"), (0x0004, "Patriarch"), (0x0008, "Deleted"),
    (0x0010, "OLEShape"), (0x0020, "HaveMaster"), (0x0040, "FlipH"), (0x0080, "FlipV"),
    (0x0100, "Connector"), (0x0200, "HaveAnchor"), (0x0400, "Background"), (0x0800, "HaveSpt"),
]

CROP_IDS = {256: "top", 257: "bottom", 258: "left", 259: "right"}


def signed(v):
    return v - 2**32 if v >= 2**31 else v


def children(buf, off, end):
    while off + 8 <= end:
        vi, rt, rl = struct.unpack_from("<HHI", buf, off)
        body = off + 8
        stop = end if rl > end - body else body + rl
        yield (vi & 0x0F, vi >> 4, rt, body, stop)
        off = body + rl
        if rl == 0 and rt == 0:
            break


def properties(buf, body, stop, count):
    props = {}
    p = body
    for _ in range(count):
        if p + 6 > stop:
            break
        pid, val = struct.unpack_from("<HI", buf, p)
        p += 6
        props[pid & 0x3FFF] = val
    return props


def find_containers(data):
    """(offset, length) of every validated OfficeArtSpContainer in a buffer."""
    out = []
    off = 0
    while off + 16 <= len(data):
        vi, rt, rl = struct.unpack_from("<HHI", data, off)
        if rt == SP_CONTAINER and (vi & 0x0F) == 0x0F and 8 <= rl <= len(data) - off - 8:
            _v2, r2, l2 = struct.unpack_from("<HHI", data, off + 8)
            if r2 == SP and l2 == 8:
                out.append((off, 8 + rl))
                off += 8 + rl
                continue
        off += 1
    return out


def picf_before(data, off):
    """The PICF whose cbHeader lands exactly on `off`, or None."""
    for k in range(58, 260):
        start = off - k
        if start < 0:
            break
        lcb, cbHeader = struct.unpack_from("<IH", data, start)
        if cbHeader != k:
            continue
        mm = struct.unpack_from("<H", data, start + 6)[0]
        if mm not in (0x63, 0x64, 0x66, 0x65, 0x62, 0x61, 0x60, 8, 99, 100, 102):
            continue
        f = struct.unpack_from("<HHH", data, start + 8)  # xExt yExt hMF
        goal = struct.unpack_from("<hhHH", data, start + 28)  # dxaGoal dyaGoal mx my
        crop = struct.unpack_from("<hhhh", data, start + 36)
        return {
            "start": start, "lcb": lcb, "cbHeader": cbHeader, "mm": mm,
            "xExt": f[0], "yExt": f[1], "hMF": f[2],
            "dxaGoal": goal[0], "dyaGoal": goal[1], "mx": goal[2], "my": goal[3],
            "dxaCropLeft": crop[0], "dyaCropTop": crop[1],
            "dxaCropRight": crop[2], "dyaCropBottom": crop[3],
        }
    return None


def describe(data, off, length):
    """Everything readable about one SpContainer, plus what follows it."""
    info = {"props": {}, "sptype": None, "spid": None, "flags": 0, "records": [], "blip": None}
    end = off + length
    for _v, inst, rt, b, s in children(data, off + 8, end):
        info["records"].append(hex(rt))
        if rt == SP:
            info["sptype"] = inst
            info["spid"], info["flags"] = struct.unpack_from("<II", data, b)
        elif rt in (OPT, OPT2, OPT3):
            info["props"].update(properties(data, b, s, inst))

    # The inline container is SpContainer + BStoreContainer; the BLIP is inside the BSE.
    if end + 8 <= len(data):
        _vi, rt, rl = struct.unpack_from("<HHI", data, end)
        if rt == BSTORE:
            for _v, _i, r2, b2, s2 in children(data, end + 8, end + 8 + rl):
                if r2 == BSE:
                    win = data[b2 + 1]
                    info["blip"] = BLIP_TYPE_BY_WIN.get(win, f"win{win}")
                    for _v3, _i3, r3, _b3, _s3 in children(data, b2 + 36, s2):
                        if r3 in BLIP_NAMES:
                            info["blip"] = BLIP_NAMES[r3]
                            break
                    break
        elif rt in BLIP_NAMES:
            info["blip"] = BLIP_NAMES[rt]
    return info


def flagnames(flags):
    return "|".join(n for m, n in SHAPE_FLAGS if flags & m) or "none"


def main():
    for arg in sys.argv[1:]:
        path = Path(arg)
        ole = olefile.OleFileIO(str(path))
        if not ole.exists("Data"):
            print(f"{path.name}: no Data stream")
            continue
        data = ole.openstream("Data").read()
        containers = find_containers(data)
        print(f"== {path.name}: Data {len(data)} bytes, {len(containers)} inline containers")
        for i, (off, length) in enumerate(containers):
            info = describe(data, off, length)
            picf = picf_before(data, off)
            crops = {CROP_IDS[k]: signed(v) / 65536.0
                     for k, v in info["props"].items() if k in CROP_IDS and signed(v)}
            pib = info["props"].get(260, 0)
            pibflags = info["props"].get(262, 0)
            line = (f"  [{i:3d}] off={off} len={length} blip={info['blip']} "
                    f"sptype={info['sptype']} spid={info['spid']} "
                    f"flags=0x{info['flags']:x}({flagnames(info['flags'])}) "
                    f"pib={pib} pibFlags=0x{pibflags:x}")
            if crops:
                line += "  CROP " + " ".join(f"{k}={v:.4f}" for k, v in crops.items())
            print(line)
            if picf:
                print(f"        PICF@{picf['start']} lcb={picf['lcb']} cbHeader={picf['cbHeader']} "
                      f"mm=0x{picf['mm']:x} xExt={picf['xExt']} yExt={picf['yExt']} hMF={picf['hMF']} "
                      f"goal={picf['dxaGoal']}x{picf['dyaGoal']} mx={picf['mx']} my={picf['my']} "
                      f"picfcrop={picf['dxaCropLeft']},{picf['dyaCropTop']},"
                      f"{picf['dxaCropRight']},{picf['dyaCropBottom']}")
            else:
                print("        PICF: not found")
            print(f"        records: {' '.join(info['records'])}")


if __name__ == "__main__":
    main()
