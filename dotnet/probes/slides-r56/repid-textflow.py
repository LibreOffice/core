#!/usr/bin/env python3
"""Turn a shape's `WrapText` (133) property entry into a `txflTextFlow` (136) one, in place.

Adding a property to an Escher `msofbtOPT` means growing the record and every container above
it; REWRITING one entry's identifier and value does not, so the file stays byte-identical
apart from six bytes per shape.  `WrapText` is the safe donor here: the fixture's shapes state
it explicitly as `mso_wrapSquare` (0), which is also the value both readers use when the
property is absent, so dropping it changes nothing.

This exists because the reference's own `.ppt` exporter will not write `txflTextFlow` for an
authored flat-ODF box -- `escherex.cxx:730` gates it on `WritingMode_TB_RL`, which
`style:writing-mode` on a `draw:frame` does not reach -- and a fixture that cannot state the
property under test cannot test it.

    repid-textflow.py <in.ppt> <out.ppt> <value>
"""
import struct, sys
import olefile

DONOR = 133
TXFL = 136


def rewrite(buf, value):
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

    donors = []

    def walk(off, end):
        for ver, inst, rt, b, s in children(off, end):
            if rt == 0xF004:
                slots = {}
                for _v2, i2, r2, b2, s2 in children(b, s):
                    if r2 in (0xF00B, 0xF121, 0xF122):
                        p = b2
                        for _ in range(i2):
                            if p + 6 > s2:
                                break
                            pid, val = struct.unpack_from("<HI", buf, p)
                            slots[pid & 0x3FFF] = (p, val)
                            p += 6
                # a shape that carries text of its own AND states the donor property
                if DONOR in slots and 128 in slots and 129 in slots:
                    donors.append(slots[DONOR][0])
            elif ver == 0x0F or rt in (0xF003, 0xF002):
                walk(b, s)

    walk(0, len(buf))

    # The fixture's own three boxes are the last three such shapes in stream order; everything
    # before them belongs to the master and the layout placeholders.
    for at in donors[-3:]:
        struct.pack_into("<HI", buf, at, TXFL, value)
        hits += 1
    return bytes(buf), hits


if __name__ == "__main__":
    src, dst, value = sys.argv[1], sys.argv[2], int(sys.argv[3])
    raw = open(src, "rb").read()
    ole = olefile.OleFileIO(src)
    entry = [e for e in ole.listdir() if e[-1].lower() == "powerpoint document"][0]
    stream = ole.openstream(entry).read()
    new, hits = rewrite(stream, value)
    at = raw.find(stream)
    if at < 0 or hits == 0:
        raise SystemExit(f"nothing rewritten in {src} (hits={hits}, contiguous={at >= 0})")
    out = bytearray(raw)
    out[at:at + len(stream)] = new
    open(dst, "wb").write(bytes(out))
    print(f"{dst}: {hits} shapes, WrapText -> txflTextFlow = {value}")
