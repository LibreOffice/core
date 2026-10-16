#!/usr/bin/env python3
"""Census of `.ppt` table groups, and of the text kinds inside them.

A `.ppt` states a table on the GROUP shape's tertiary property table
(`msofbtUDefProp`): `DFF_Prop_tableProperties` (927) with either of its low two
bits set, and `DFF_Prop_tableRowProperties` (928) as a complex value.  The
reference then replaces the whole group with one `SdrTableObj`
(`svdfppt.cxx`:2913, `CreateTable` at :7569), which copies each child's
`OutlinerParaObject` into a cell and NOTHING else of its item set --
`ApplyCellAttributes` (:7412) carries the text distances, the two adjusts, the
writing mode and the fill, and not `SDRATTR_TEXT_FITTOSIZE`.

So a cell never autofits, whatever the child rectangle's TextHeaderAtom said.
This counts the shapes that changes.

Usage: tablecensus.py <corpus-root> [glob]
"""
import struct, sys, pathlib, collections
import olefile

TABLE_PROPS = 927
TABLE_ROWS = 928
# TextHeaderAtom kinds that svdfppt.cxx:1030-1099 gives AUTOFIT.
AUTOFIT_KINDS = {1, 7, 8}          # Body, HalfBody, QuarterBody


def records(data, off, end, depth=0):
    """Walk the DFF/PPT record tree; both use the same eight-byte header."""
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
    """The six-byte fixed entries of a property table."""
    out = {}
    for i in range(min(count, length // 6)):
        raw, val = struct.unpack_from('<HI', data, body + i * 6)
        out[raw & 0x3FFF] = (val, bool(raw & 0x8000))
    return out


def scan(path):
    """(table groups, autofit-kind text shapes inside them) for one file."""
    with olefile.OleFileIO(str(path)) as f:
        if not f.exists('PowerPoint Document'):
            return 0, 0, 0
        data = f.openstream('PowerPoint Document').read()

    groups = cells = texted = 0
    # An SpContainer (0xF004) that declares the table properties is a group's own
    # descriptor shape; its siblings inside the enclosing SpgrContainer are the cells.
    spgr_stack = []
    tables = []
    for typ, inst, body, ln, depth in records(data, 0, len(data)):
        if typ == 0xF122:
            p = props(data, body, inst, ln)
            if p.get(TABLE_PROPS, (0, False))[0] & 3 and TABLE_ROWS in p:
                groups += 1
                tables.append(depth)

    # Count the text shapes of every table group: walk again, tracking the
    # enclosing SpgrContainer and whether its first SpContainer declared a table.
    def walk(off, end, in_table):
        nonlocal cells, texted
        while off + 8 <= end:
            ver, typ, ln = struct.unpack_from('<HHI', data, off)
            body, bend = off + 8, off + 8 + ln
            if bend > end:
                return
            if typ == 0xF003:                      # SpgrContainer
                walk(body, bend, table_group(body, bend))
            elif typ == 0xF004 and in_table:       # a member of a table group
                cells += 1
                if kind_of(body, bend) in AUTOFIT_KINDS:
                    texted += 1
            elif (ver & 0xF) == 0xF:
                walk(body, bend, False)
            off = bend

    def table_group(off, end):
        """Whether the first SpContainer of this SpgrContainer declares a table."""
        while off + 8 <= end:
            ver, typ, ln = struct.unpack_from('<HHI', data, off)
            body, bend = off + 8, off + 8 + ln
            if bend > end:
                return False
            if typ == 0xF004:
                for t2, i2, b2, l2, _ in records(data, body, bend):
                    if t2 == 0xF122:
                        p = props(data, b2, i2, l2)
                        return bool(p.get(TABLE_PROPS, (0, False))[0] & 3) and TABLE_ROWS in p
                return False
            off = bend
        return False

    def kind_of(off, end):
        """The TextHeaderAtom kind inside an SpContainer's client textbox, or -1."""
        for typ, inst, body, ln, _ in records(data, off, end):
            if typ == 3999 and ln >= 4:            # TextHeaderAtom
                return struct.unpack_from('<I', data, body)[0]
        return -1

    walk(0, len(data), False)
    return groups, cells, texted


def main():
    root = pathlib.Path(sys.argv[1])
    pattern = sys.argv[2] if len(sys.argv) > 2 else '**/*.ppt'
    total = collections.Counter()
    for path in sorted(root.glob(pattern)):
        try:
            groups, cells, texted = scan(path)
        except Exception as exc:                    # a corpus file may be anything
            print(f'{path.name}\tERROR\t{exc}')
            continue
        if groups:
            print(f'{path.name}\tgroups {groups}\tcells {cells}\tautofit-kind {texted}')
            total['documents'] += 1
            total['groups'] += groups
            total['cells'] += cells
            total['texted'] += texted
    print('TOTAL', dict(total))


if __name__ == '__main__':
    main()
