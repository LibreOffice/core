#!/usr/bin/env python3
"""For every `.ppt` Body-kind placeholder that states `wrapNone`: does it carry an
OEPlaceholderAtom with a real id, and what shape type does it declare?

That is the whole of the exception in `svdfppt.cxx`:1043-1055. `bAutoGrowWidth = !bWordWrap`
is reached only when the object LibreOffice built is an `SdrObjCustomShape` **and** the text
kind had been rewritten to Rectangle, which happens only when the shape carries no
`OEPlaceholderAtom` or one whose id is NONE (`:1043-1047`). A Body placeholder that names
itself as one takes `bAutoGrowWidth = false` (`:1084`) and the wrap decides nothing.
"""
import struct, sys, pathlib, collections
import olefile, census
from persist import directory, current_user_edit
from wrap_shim import shapes, properties, WRAPTEXT, FITTEXTTOSHAPE, BODY_KINDS

CLIENTDATA, SP = 0xF011, 0xF00A
OEPLACEHOLDER = 3011

def placeholder(buf, sp):
    for v, rt, b, st in census.records(buf, sp[0], sp[1]):
        if rt != CLIENTDATA: continue
        for v2, t2, b2, s2 in census.records(buf, b, st):
            if t2 == OEPLACEHOLDER and s2 - b2 >= 5:
                return buf[b2 + 4]
    return None

def shapetype(buf, sp):
    for v, rt, b, st in census.records(buf, sp[0], sp[1]):
        if rt == SP:
            return v >> 4          # instance is the msosptXxx shape type
    return None

def main(root):
    tally = collections.Counter()
    for p in sorted(q for q in pathlib.Path(root).rglob('*')
                    if q.suffix.lower() == '.ppt' and q.is_file()):
        ole = olefile.OleFileIO(str(p))
        if not ole.exists('PowerPoint Document'): ole.close(); continue
        buf = ole.openstream('PowerPoint Document').read()
        cur = current_user_edit(ole); ole.close()
        offs, doc = directory(buf, cur)
        roots = census.live_roots(buf, offs, doc)
        for kinds, sp in shapes(buf, roots):
            if not any(k in BODY_KINDS for k in kinds): continue
            props = properties(buf, sp)
            if props.get(WRAPTEXT, 0) != 2: continue
            ph = placeholder(buf, sp)
            grows = (props.get(FITTEXTTOSHAPE, 0) & 2) != 0
            tally[(p.name, ph, shapetype(buf, sp), grows)] += 1
    for k, n in sorted(tally.items()):
        print(f'{k[0]:40s} placeholder={k[1]} shapetype={k[2]} fitShapeToText={k[3]}  x{n}')
    print('total', sum(tally.values()))

if __name__ == '__main__':
    main(sys.argv[1])
