#!/usr/bin/env python3
"""Rewrite a .ppt's TextHeaderAtom instances in place, so an authored deck can be given the
text KIND that turns the binary importer's autofit on.

`soffice --convert-to ppt` does not preserve `a:normAutofit`: autofit is not spelled
anywhere in the binary format.  `svdfppt.cxx:1030-1039` infers it from the TextHeaderAtom's
instance -- Body, HalfBody or QuarterBody get `TextFitToSizeType_AUTOFIT` and every other
kind gets none.  A round-tripped text box comes out instance 4 (TextInShape) and is never
fitted, which is what the first cut of this probe measured: 40 pt over 21 overflowing lines
on every slide.

This flips the chosen atoms to instance 1 (Body).  The record body is a single uint32 and
the edit is length-preserving, so no container length has to move.

    ppt-patch-kind.py <in.ppt> <out.ppt> [--kind 1] [--skip-first-per-slide]
"""
import argparse, shutil, struct
import olefile

TEXT_HEADER_ATOM = 3999
SLIDE = 1006


def records(data, start, end):
    pos = start
    while pos + 8 <= end:
        ver_inst, rtype, length = struct.unpack_from("<HHI", data, pos)
        body = pos + 8
        stop = min(body + length, end)
        yield (ver_inst & 0x0F, ver_inst >> 4, rtype, body, stop)
        pos = stop
        if length == 0 and rtype == 0:
            break


def walk(data, start, end):
    for ver, inst, rtype, body, stop in records(data, start, end):
        yield (ver, inst, rtype, body, stop)
        if ver == 0x0F:
            yield from walk(data, body, stop)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src")
    ap.add_argument("dst")
    ap.add_argument("--kind", type=int, default=1, help="TSS_Type to write (1 = Body)")
    ap.add_argument("--skip-first-per-slide", action="store_true",
                    help="leave the first text object on each slide alone (the spacer)")
    a = ap.parse_args()

    shutil.copyfile(a.src, a.dst)
    ole = olefile.OleFileIO(a.dst, write_mode=True)
    data = bytearray(ole.openstream("PowerPoint Document").read())

    slides = [(b, e) for ver, inst, rtype, b, e in walk(data, 0, len(data))
              if rtype == SLIDE and ver == 0x0F]
    changed = 0
    for b, e in slides:
        heads = [bb for ver, inst, rtype, bb, ee in walk(data, b, e)
                 if rtype == TEXT_HEADER_ATOM]
        if a.skip_first_per_slide:
            heads = heads[1:]
        for bb in heads:
            before, = struct.unpack_from("<I", data, bb)
            struct.pack_into("<I", data, bb, a.kind)
            changed += 1
            if changed <= 3:
                print(f"  @{bb}: instance {before} -> {a.kind}")
    ole.write_stream("PowerPoint Document", bytes(data))
    ole.close()
    print(f"{a.dst}: {len(slides)} slides, {changed} TextHeaderAtoms set to {a.kind}")


if __name__ == "__main__":
    main()
