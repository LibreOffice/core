#!/usr/bin/env python3
"""How many `.ppt` outline placeholders state `wrapNone`?

`Autofits` requires `Wraps(shape)`, and that is an approximation the seat's own doc-comment
records: `svdfppt.cxx`:1053-1055 derives `bAutoGrowWidth = !bWordWrap` **only** for a custom
shape whose text kind resolved to Rectangle, while every other branch sets
`bAutoGrowWidth = false` (`:1084`) -- so a real Body/HalfBody/QuarterBody placeholder is
autofitted whatever its wrap says.  This counts the shapes on which the two rules disagree.
"""
import struct, sys, pathlib
import olefile, census
from persist import directory, current_user_edit

CLIENTTEXTBOX, SPCONTAINER, OPT = 0xF00D, 0xF004, 0xF00B
TEXTHEADER = 3999
WRAPTEXT, FITTEXTTOSHAPE = 133, 191
BODY_KINDS = {1, 7, 8}          # Body, HalfBody, QuarterBody -- exactly what `Autofits` admits

def properties(buf, sp):
    """The shape's Escher property table, id -> value."""
    out = {}
    for ver, rt, b, st in census.records(buf, sp[0], sp[1]):
        if rt != OPT: continue
        count = struct.unpack_from('<H', buf, b - 6)[0] >> 4
        p = b
        for _ in range(count):
            if p + 6 > st: break
            pid, val = struct.unpack_from('<HI', buf, p)
            out[pid & 0x3FFF] = val
            p += 6
    return out

def shapes(buf, roots):
    for rt, off, end in roots:
        yield from walk(buf, off + 8, end)

def walk(buf, s, e):
    for ver, rt, b, st in census.records(buf, s, e):
        if rt == SPCONTAINER:
            kinds = []
            for v2, t2, b2, s2 in census.records(buf, b, st):
                if t2 != CLIENTTEXTBOX: continue
                for v3, t3, b3, s3 in census.records(buf, b2, s2):
                    if t3 == TEXTHEADER and s3 - b3 >= 4:
                        kinds.append(struct.unpack_from('<I', buf, b3)[0])
            if kinds: yield (kinds, (b, st))
        if ver == 0x0F:
            yield from walk(buf, b, st)

def main(root):
    body = wrapless = growing = 0
    docs = set()
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
            body += 1
            props = properties(buf, sp)
            if props.get(WRAPTEXT, 0) == 2:      # mso_wrapNone
                wrapless += 1
                docs.add(p.name)
                if (props.get(FITTEXTTOSHAPE, 0) & 2) == 0: growing += 1
    print(f'{body} body-kind shapes in the 51 .ppt; {wrapless} state wrapNone '
          f'({len(docs)} documents), of which {growing} would newly autofit')
    for d in sorted(docs): print('   ', d)

if __name__ == '__main__':
    main(sys.argv[1])
