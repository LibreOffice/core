#!/usr/bin/env python3
"""Every shape in a .ppt whose Escher property 255 has the fGtext bit (0x4000) set.

`filter/source/msfilter/msdffimp.cxx`:4423-4426 makes a shape WordArt on that bit alone --
`( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x4000 ) != 0` -- and not on the
shape type being one of the forty text-path presets. So this counts both: how many shapes
carry the bit, and how many of those are outside 136..175, where
`EnhancedCustomShapeTypeNames::Get` names no `fontwork-*` preset.

    fgtext.py <file.ppt>...
"""
import struct, sys
import olefile

SP = 0xF00A
OPT = 0xF00B
SPC = 0xF004


def walk(s, a, b, path, out):
    p = a
    while p + 8 <= b:
        vi, rt, rl = struct.unpack_from('<HHI', s, p)
        out.append((rt, vi >> 4, p, rl, tuple(path)))
        if (vi & 0x0F) == 0x0F and p + 8 + rl <= b:
            walk(s, p + 8, p + 8 + rl, path + [(rt, p)], out)
        p += 8 + rl


def properties(s, p, count):
    props = {}
    for i in range(count):
        q = p + 8 + i * 6
        if q + 6 > len(s):
            break
        pid, val = struct.unpack_from('<HI', s, q)
        props.setdefault(pid & 0x3FFF, val)
    return props


def main(paths):
    total = inrange = outrange = 0
    for path in paths:
        try:
            ole = olefile.OleFileIO(path)
            s = ole.openstream('PowerPoint Document').read()
            ole.close()
        except Exception as e:
            print(f'{path}\tERROR {e}')
            continue
        out = []
        walk(s, 0, len(s), [], out)
        for rt, inst, p, rl, parents in out:
            if rt != SP:
                continue
            container = parents[-1] if parents else None
            if not (container and container[0] == SPC):
                continue
            start = container[1] + 8
            end = start + struct.unpack_from('<I', s, container[1] + 4)[0]
            props = {}
            for r2, i2, p2, l2, _ in out:
                if r2 == OPT and start <= p2 < end:
                    for k, v in properties(s, p2, i2).items():
                        props.setdefault(k, v)
            if not (props.get(255, 0) & 0x4000):
                continue
            total += 1
            band = 136 <= inst < 176
            inrange += band
            outrange += not band
            print(f'{path.rsplit("/", 1)[-1]}\tsp@{p}\ttype={inst}\t'
                  f'{"text-path type" if band else "OUTSIDE 136..175"}\t'
                  f'gtextUNICODE={"yes" if 192 in props else "no"}')
    print(f'# shapes with fGtext: {total}   in 136..175: {inrange}   outside: {outrange}')


if __name__ == '__main__':
    main(sys.argv[1:])
