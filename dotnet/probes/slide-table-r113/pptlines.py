#!/usr/bin/env python3
"""What a `.ppt` table group's LINE members state for their width, and what the
reference makes of it.

A `.ppt` writes a table as a group of rectangles plus a set of LINE shapes; the
reference (`svdfppt.cxx`:7658-7676) throws the rectangles' item sets away and turns
each line member into cell borders through `ApplyCellLineAttributes` (:7513), which
sets

    BorderLine2.LineWidth = max(1, XATTR_LINEWIDTH / 4)          -- INTEGER divide

with `XATTR_LINEWIDTH` in 1/100 mm (`msdffimp.cxx`:1048-1049, `ScaleEmu` = EMU/360
for a Map100thMM model).  The table's view contact then hands that number to
`svx::frame::Style` scaled by `o3tl::convert(1.0, twip, mm100)` = 2540/1440
(`viewcontactoftableobj.cxx`:183-184), i.e. it reads the 1/100 mm value as TWIPS.

So this prints, per group, the EMU the file states and the width the reference will
draw, so the prediction can be checked against its PDF.

Usage: pptlines.py <file.ppt> ...
"""
import struct, sys, pathlib, collections
import olefile

TABLE_PROPS, TABLE_ROWS = 927, 928
PROP_LINEWIDTH = 459           # DFF_Prop_lineWidth, EMU, default 9525
PROP_NOLINEDRAWDASH = 511      # DFF_Prop_fNoLineDrawDash


def records(data, off, end, depth=0):
    while off + 8 <= end:
        ver, typ, ln = struct.unpack_from('<HHI', data, off)
        body, bend = off + 8, off + 8 + ln
        if bend > end:
            return
        yield typ, ver >> 4, body, ln, depth
        if (ver & 0xF) == 0xF:
            yield from records(data, body, bend, depth + 1)
        off = bend


def props(data, body, count, length):
    out = {}
    for i in range(min(count, length // 6)):
        raw, val = struct.unpack_from('<HI', data, body + i * 6)
        out[raw & 0x3FFF] = val
    return out


def shape_props(data, off, end):
    """The first msofbtOPT (0xF00B) of one SpContainer, and its Sp record."""
    p, shtype, flags = {}, -1, 0
    for typ, inst, body, ln, _ in records(data, off, end):
        if typ == 0xF00A and ln >= 8:
            shtype = inst
            flags = struct.unpack_from('<I', data, body + 4)[0]
        elif typ == 0xF00B and not p:
            p = props(data, body, inst, ln)
    return shtype, flags, p


def group_is_table(data, off, end):
    while off + 8 <= end:
        ver, typ, ln = struct.unpack_from('<HHI', data, off)
        body, bend = off + 8, off + 8 + ln
        if bend > end:
            return False
        if typ == 0xF004:
            for t2, i2, b2, l2, _ in records(data, body, bend):
                if t2 == 0xF122:
                    p = props(data, b2, i2, l2)
                    return bool(p.get(TABLE_PROPS, 0) & 3) and TABLE_ROWS in p
            return False
        off = bend
    return False


def emu_to_ref_pt(emu):
    """EMU -> what 26.2.4.2 finally strokes, in PDF points."""
    mm100 = int(emu / 360)                       # msdffimp.cxx ScaleEmu, truncating
    border = max(1, mm100 // 4)                  # svdfppt.cxx ApplyCellLineAttributes
    return border * 2540.0 / 1440.0 * 72.0 / 2540.0, mm100, border


def scan(path):
    with olefile.OleFileIO(str(path)) as f:
        if not f.exists('PowerPoint Document'):
            return []
        data = f.openstream('PowerPoint Document').read()

    out = []

    def walk(off, end, in_table, gid):
        while off + 8 <= end:
            ver, typ, ln = struct.unpack_from('<HHI', data, off)
            body, bend = off + 8, off + 8 + ln
            if bend > end:
                return
            if typ == 0xF003:
                t = group_is_table(data, body, bend)
                walk(body, bend, t, off if t else gid)
            elif typ == 0xF004 and in_table:
                shtype, flags, p = shape_props(data, body, bend)
                # msoLine = 20; the reference's IsLine() is a path with two points,
                # which for these files is always shape type 20.
                if shtype == 20:
                    emu = p.get(PROP_LINEWIDTH, 9525)
                    out.append((gid, emu) )
            elif (ver & 0xF) == 0xF:
                walk(body, bend, False, gid)
            off = bend

    walk(0, len(data), False, 0)
    return out


def main():
    for name in sys.argv[1:]:
        rows = scan(pathlib.Path(name))
        per = collections.Counter(emu for _, emu in rows)
        print(f'{pathlib.Path(name).name}\tline members {len(rows)}\tgroups {len({g for g,_ in rows})}')
        for emu, n in sorted(per.items()):
            pt, mm100, border = emu_to_ref_pt(emu)
            print(f'  {n:5} x {emu:9} EMU = {emu/12700:6.3f} pt'
                  f'  -> mm100 {mm100:4}  -> border {border:4}  -> reference draws {pt:.4f} pt')


if __name__ == '__main__':
    main()
