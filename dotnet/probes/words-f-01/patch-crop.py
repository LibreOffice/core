#!/usr/bin/env python3
"""Rewrite one inline picture's Escher crop, in place, and render the result.

    patch-crop.py <in.doc> <out.doc> <container-index> <edge>=<fraction> [...]

The value is a 4-byte fixed-point 16.16 field inside an OfficeArtFOPT entry, so the edit
changes no length and no offset anywhere in the file — the same discipline
`crop-wiring-01` used to build `picture-crop-goal.doc`. Only an existing property is
rewritten; nothing is inserted, because inserting would move every later offset and the
piece table would no longer point at the pictures.

This is the instrument for the one question the crop round left open: **what decides
whether LibreOffice applies an Escher crop to an inline `.doc` picture?** The candidates
that survive a census of `150_5300_13_chg10` are the blip's kind and the crop's
magnitude, and they are separated by moving one and holding the other.
"""
import struct
import sys
from pathlib import Path

import olefile

SP = 0xF00A
SPC = 0xF004
OPT_RECORDS = (0xF00B, 0xF121, 0xF122)
EDGES = {"top": 256, "bottom": 257, "left": 258, "right": 259}


def containers(data):
    out, off = [], 0
    while off + 16 <= len(data):
        vi, rt, rl = struct.unpack_from("<HHI", data, off)
        if rt == SPC and (vi & 0x0F) == 0x0F and 8 <= rl <= len(data) - off - 8:
            _v, r2, l2 = struct.unpack_from("<HHI", data, off + 8)
            if r2 == SP and l2 == 8:
                out.append((off, 8 + rl))
                off += 8 + rl
                continue
        off += 1
    return out


def property_offsets(data, off, length):
    """{property id: absolute offset of its 4-byte value} for one SpContainer."""
    found = {}
    p = off + 8
    end = off + length
    while p + 8 <= end:
        vi, rt, rl = struct.unpack_from("<HHI", data, p)
        body = p + 8
        if rt in OPT_RECORDS:
            q = body
            for _ in range((vi >> 4)):
                if q + 6 > body + rl:
                    break
                pid = struct.unpack_from("<H", data, q)[0] & 0x3FFF
                found.setdefault(pid, q + 2)
                q += 6
        p = body + rl
    return found


def main():
    src, dst, index = Path(sys.argv[1]), Path(sys.argv[2]), int(sys.argv[3])
    edits = [a.split("=") for a in sys.argv[4:]]

    raw = bytearray(src.read_bytes())
    ole = olefile.OleFileIO(str(src))
    data = ole.openstream("Data").read()

    off, length = containers(data)[index]
    offsets = property_offsets(data, off, length)

    # The Data stream is not necessarily contiguous in the file, so each edited word is
    # located by finding the stream slice it sits in. A 24-byte window around the value is
    # unique in every file this was run on, and the script refuses rather than guessing.
    for edge, value in edits:
        pid = EDGES[edge]
        if pid not in offsets:
            sys.exit(f"container {index} states no {edge} crop; refusing to insert one")
        at = offsets[pid]
        old = data[at - 2:at + 4]
        window = data[max(0, at - 12):at + 16]
        hits = [i for i in range(len(raw)) if raw[i:i + len(window)] == window]
        if len(hits) != 1:
            sys.exit(f"{len(hits)} matches for the {edge} window; refusing")
        base = hits[0] + (at - max(0, at - 12))
        new = int(round(float(value) * 65536.0))
        print(f"  {edge}: {struct.unpack_from('<I', data, at)[0] / 65536.0:.4f} -> "
              f"{new / 65536.0:.4f}  at file offset {base}")
        struct.pack_into("<I", raw, base, new)
        assert raw[base - 2:base + 4] != old or new == struct.unpack_from("<I", data, at)[0]

    dst.write_bytes(bytes(raw))
    print(f"wrote {dst}")


if __name__ == "__main__":
    main()
