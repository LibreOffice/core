#!/usr/bin/env python3
"""Every Escher WordArt shape in a .ppt: the shape type, the text it carries, and its fill.

An Escher WordArt is an autoshape whose *type* is one of the text-path presets --
`mso_sptTextPlainText` (136) through `mso_sptTextCanDown` (175) -- and
`EnhancedCustomShapeTypeNames::Get` maps that number onto the `fontwork-*` name the drawing
layer builds its geometry from (`svx/source/customshapes/EnhancedCustomShapeTypeNames.cxx`).
The shape type is the `msofbtSp` record's *instance* field, so this needs nothing but the
record tree.

Reported beside it: `gtextUNICODE` (property 192 in the shape's `msofbtOPT`) is the WordArt's
own text, and 385/386/447/448 say whether the glyphs are filled with a colour or with a
picture -- which is the half of the seat a solid `Paint` cannot draw.

    wordart.py <file.ppt>...
"""
import struct, sys
import olefile

SP = 0xF00A          # msofbtSp -- instance is the shape type
OPT = 0xF00B         # msofbtOPT
SPC = 0xF004         # msofbtSpContainer

TEXT_PATH = range(136, 176)


def walk(s, a, b, path, out):
    p = a
    while p + 8 <= b:
        vi, rt, rl = struct.unpack_from('<HHI', s, p)
        out.append((rt, vi >> 4, p, rl, tuple(path)))
        if (vi & 0x0F) == 0x0F and p + 8 + rl <= b:
            walk(s, p + 8, p + 8 + rl, path + [(rt, p)], out)
        p += 8 + rl


def properties(s, p, count):
    q = p + 8
    props = {}
    for _ in range(count):
        if q + 6 > len(s):
            break
        pid, val = struct.unpack_from('<HI', s, q)
        q += 6
        props[pid & 0x3FFF] = (val, (pid >> 15) & 1)
    return props


def main(paths):
    for path in paths:
        ole = olefile.OleFileIO(path)
        s = ole.openstream('PowerPoint Document').read()
        ole.close()
        out = []
        walk(s, 0, len(s), [], out)

        # every shape container, so a text-path Sp can be paired with the OPT beside it
        found = 0
        for rt, inst, p, rl, parents in out:
            if rt != SP or inst not in TEXT_PATH:
                continue
            container = parents[-1] if parents else None
            props = {}
            if container and container[0] == SPC:
                start, end = container[1] + 8, container[1] + 8 + \
                    struct.unpack_from('<I', s, container[1] + 4)[0]
                for r2, i2, p2, l2, _ in out:
                    if r2 == OPT and start <= p2 < end:
                        props.update(properties(s, p2, i2))
            fill = {k: v[0] for k, v in props.items() if k in (385, 386, 390, 447, 448, 191)}
            text = props.get(192)
            found += 1
            print(f'{path.rsplit("/", 1)[-1]}\tsp@{p}\ttype={inst}\tfill={fill}\t'
                  f'gtextUNICODE={"yes" if text else "no"}')
        if found == 0:
            print(f'{path.rsplit("/", 1)[-1]}\tno text-path shape')


if __name__ == '__main__':
    main(sys.argv[1:])
