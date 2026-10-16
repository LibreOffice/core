#!/usr/bin/env python3
"""Count corpus `.ppt` carrying an Escher picture-recolour property.

`DFF_Prop_pictureContrast` is 264 and `DFF_Prop_pictureBrightness` is 265
(include/svx/msdffdef.hxx). Both live in an `msofbtOPT` record (0xF00B), whose payload is a
run of 6-byte entries: a little-endian property id (bits 0-13) with two flag bits, then a
4-byte value. Not compressed — only blips are — so a straight walk of the PowerPoint
Document stream finds them.
"""
import glob
import struct
import sys

try:
    import olefile
except ImportError:
    olefile = None

OPT = 0xF00B
TERTIARY = 0xF122
SECONDARY = 0xF121
WANT = {264, 265}


def walk(buf):
    """Yield (recVer, recInstance, recType, payload) for every Escher record found."""
    n = len(buf)
    i = 0
    while i + 8 <= n:
        ver_inst, rec_type, length = struct.unpack_from("<HHI", buf, i)
        ver = ver_inst & 0x0F
        inst = ver_inst >> 4
        body = i + 8
        if length > n - body or length < 0:
            i += 1
            continue
        if ver == 0x0F:
            yield ver, inst, rec_type, b""
            i = body
        else:
            yield ver, inst, rec_type, buf[body:body + length]
            i = body + length


def found(buf):
    hits = {}
    for _ver, inst, rec_type, payload in walk(buf):
        if rec_type not in (OPT, SECONDARY, TERTIARY):
            continue
        for k in range(inst):
            off = k * 6
            if off + 6 > len(payload):
                break
            pid, value = struct.unpack_from("<Hi", payload, off)
            prop = pid & 0x3FFF
            if prop in WANT:
                hits.setdefault(prop, []).append(value)
    return hits


def main():
    files = sorted(
        f for pat in ("*.ppt", "*.PPT")
        for f in glob.glob(f"/workspace/sample-files/slides/**/{pat}", recursive=True))
    carriers = 0
    print(f"{len(files)} .ppt in the corpus")
    for path in files:
        data = open(path, "rb").read()
        streams = []
        if olefile and olefile.isOleFile(path):
            ole = olefile.OleFileIO(path)
            for entry in ole.listdir():
                if entry[-1] in ("PowerPoint Document", "Pictures"):
                    streams.append(ole.openstream(entry).read())
            ole.close()
        if not streams:
            streams = [data]
        hits = {}
        for s in streams:
            for k, v in found(s).items():
                hits.setdefault(k, []).extend(v)
        if hits:
            carriers += 1
            print("  ", path.split("/")[-1], {k: len(v) for k, v in hits.items()},
                  {k: sorted(set(v))[:6] for k, v in hits.items()})
    print(f"carriers: {carriers} of {len(files)}")


main()
