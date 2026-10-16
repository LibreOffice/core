#!/usr/bin/env python3
"""Does a .doc's PICF goal size include the crop, or has it already been taken off?

    picf-crop.py <file.doc> [...]

The whole of this round's word-path change rests on the answer. Every other host this
library reads states the rectangle the *visible* part of a picture lands in — a slide
shape's anchor, an FSPA, a sheet's client anchor — and `PictureCrop.Uncropped` grows
those. An inline .doc picture is placed from `PICF.dxaGoal` scaled by `mx`, and if that
is the *whole* picture then growing it is exactly backwards.

So the two statements of the same crop are read side by side: the PICF's own
`dxaCropLeft`/`Top`/`Right`/`Bottom`, which are twips off the goal, and the Escher
properties 256-259 on the SpContainer that follows the PICF, which are 16.16 fractions.
If `dxaCropLeft / dxaGoal` equals the Escher fraction, the goal is the whole picture and
the frame is the inset of it.

PICFs are found by scanning the Data stream for the header LibreOffice's own reader
tests for — `lcb >= 58`, `cbHeader` placing the shape inside the record, and a mapping
mode of 0x64 or 0x66, which is `SwWW8ImplReader::ImportGraf`, ww8graf2.cxx:498.
"""
import struct
import sys

import olefile


def picfs(data):
    off = 0
    while off + 0x44 <= len(data):
        lcb, = struct.unpack_from("<i", data, off)
        cb, = struct.unpack_from("<H", data, off + 4)
        mm, = struct.unpack_from("<h", data, off + 6)
        if 58 <= lcb <= len(data) - off and 0x2E <= cb <= lcb and mm in (0x64, 0x66):
            dxaGoal, dyaGoal = struct.unpack_from("<hh", data, off + 0x1C)
            mx, my = struct.unpack_from("<HH", data, off + 0x20)
            cl, ct, cr, cb2 = struct.unpack_from("<hhhh", data, off + 0x24)
            yield off, cb, dxaGoal, dyaGoal, mx, my, (cl, ct, cr, cb2)
            off += lcb
            continue
        off += 1


def escher_crop(data, at):
    """Properties 256-259 of the SpContainer at an offset, as signed fractions."""
    if at + 16 > len(data):
        return None
    vi, rt, rl = struct.unpack_from("<HHI", data, at)
    if rt != 0xF004 or (vi & 0x0F) != 0x0F:
        return None

    out, off, end = {}, at + 8, min(at + 8 + rl, len(data))
    while off + 8 <= end:
        v2, r2, l2 = struct.unpack_from("<HHI", data, off)
        if r2 in (0xF00B, 0xF121, 0xF122):
            p = off + 8
            for _ in range(v2 >> 4):
                if p + 6 > end:
                    break
                pid, val = struct.unpack_from("<HI", data, p)
                p += 6
                if (pid & 0x3FFF) in (256, 257, 258, 259):
                    out[pid & 0x3FFF] = val - 2**32 if val >= 2**31 else val
        off += 8 + l2
    return out


def main():
    for path in sys.argv[1:]:
        ole = olefile.OleFileIO(path)
        if not ole.exists("Data"):
            print(path, "no Data stream")
            continue
        data = ole.openstream("Data").read()

        print("==", path.split("/")[-1])
        for off, cb, dxa, dya, mx, my, crop in picfs(data):
            esc = escher_crop(data, off + cb) or {}
            if not esc and not any(crop):
                continue

            cl, ct, cr, cbo = crop
            print(f"  picf@{off:#x} goal={dxa}x{dya} twips  mx={mx} my={my}")
            print(f"    PICF   crop l={cl} t={ct} r={cr} b={cbo} twips"
                  + (f"  = l {cl / dxa:.4f} t {ct / dya:.4f} r {cr / dxa:.4f} "
                     f"b {cbo / dya:.4f} of the goal" if dxa and dya else ""))
            print("    ESCHER crop " + " ".join(
                f"{n}={esc.get(i, 0) / 65536.0:.4f}"
                for i, n in ((258, "l"), (256, "t"), (259, "r"), (257, "b"))))
            if dxa and dya:
                across = 1 - (esc.get(258, 0) + esc.get(259, 0)) / 65536.0
                down = 1 - (esc.get(256, 0) + esc.get(257, 0)) / 65536.0
                print(f"    goal x mx = {dxa * mx / 1000 / 20:.2f} x "
                      f"{dya * my / 1000 / 20:.2f} pt; inset by the Escher crop = "
                      f"{dxa * mx / 1000 / 20 * across:.2f} x "
                      f"{dya * my / 1000 / 20 * down:.2f} pt; PICF display = "
                      f"{(dxa - cl - cr) * mx / 1000 / 20:.2f} x "
                      f"{(dya - ct - cbo) * my / 1000 / 20:.2f} pt")


if __name__ == "__main__":
    main()
