#!/usr/bin/env python3
"""Rewrite every Escher `txflTextFlow` (136) value in a .ppt, in place, to a stated number.

A DISCRIMINATING fixture rather than a confirming one, and deliberately not a round trip:
`soffice --convert-to ppt` writes the property only for a shape it has already decided is
vertical, so a fixture built that way states the reference's own answer and cannot separate
the six `MSO_TXFL` values from each other.  Patching the four value bytes of a real corpus
document leaves every other byte identical, so a rendering difference between two arms is
the property and nothing else.

Six arms, and the three candidate rules give three different answers:

  value                      H1 "1,3,5 vertical; 2 rotates"   H2 "any non-zero turns"  H3 "only 1"
  0 HorzN / 4 HorzA          upright                          upright                  upright
  1 TtoBA                    clockwise quarter                clockwise quarter        clockwise
  2 BtoT                     ANTIclockwise quarter            clockwise quarter        upright
  3 TtoBN / 5 VertN          clockwise quarter                clockwise quarter        upright

    patch-textflow.py <in.ppt> <out.ppt> <value> [only-current]

`only-current` restricts the rewrite to entries that currently hold that value, which is what
keeps the family single-variable: `concepts-surrounding-cloud-computing…ppt` states the
property on 106 shapes, 104 of them explicitly `HorzN`, and rewriting those too would change
the page for a reason that has nothing to do with the arm.
"""
import struct, sys
import olefile

TXFL = 136


def patched(buf, value, only=None):
    buf = bytearray(buf)
    hits = 0

    def children(off, end):
        while off + 8 <= end:
            vi, rt, rl = struct.unpack_from("<HHI", buf, off)
            body = off + 8
            stop = min(body + rl, end)
            if rl > len(buf):
                return
            yield (vi & 0x0F, vi >> 4, rt, body, stop)
            off = body + rl

    def walk(off, end):
        nonlocal hits
        for ver, inst, rt, b, s in children(off, end):
            if rt in (0xF00B, 0xF121, 0xF122):
                p = b
                for _ in range(inst):
                    if p + 6 > s:
                        break
                    pid, _v = struct.unpack_from("<HI", buf, p)
                    if (pid & 0x3FFF) == TXFL and (only is None or _v == only):
                        struct.pack_into("<I", buf, p + 2, value)
                        hits += 1
                    p += 6
            elif ver == 0x0F or rt in (0xF003, 0xF002, 0xF004):
                walk(b, s)

    walk(0, len(buf))
    return bytes(buf), hits


if __name__ == "__main__":
    src, dst, value = sys.argv[1], sys.argv[2], int(sys.argv[3])
    only = int(sys.argv[4]) if len(sys.argv) > 4 else None
    raw = open(src, "rb").read()
    ole = olefile.OleFileIO(src)
    entry = None
    for e in ole.listdir():
        if e[-1].lower() == "powerpoint document":
            entry = e
    stream = ole.openstream(entry).read()
    new, hits = patched(stream, value, only)
    if hits == 0:
        raise SystemExit(f"no txflTextFlow in {src}")
    # The stream sits verbatim in the container for a document this size; rewrite by search.
    at = raw.find(stream)
    if at < 0:
        raise SystemExit("stream is not contiguous in the container — needs a real OLE writer")
    out = bytearray(raw)
    out[at:at + len(stream)] = new
    open(dst, "wb").write(bytes(out))
    print(f"{dst}: {hits} txflTextFlow -> {value}")
