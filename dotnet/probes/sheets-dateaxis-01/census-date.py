#!/usr/bin/env python3
"""Count chart substreams whose category axis states a BIFF date axis."""
import os, struct, sys
import olefile

def recs(d):
    pos = 0
    while pos + 4 <= len(d):
        rid, ln = struct.unpack_from('<HH', d, pos)
        pos += 4
        yield rid, d[pos:pos+ln]
        pos += ln

def scan(path):
    try:
        if not olefile.isOleFile(path):
            return None
        o = olefile.OleFileIO(path)
    except Exception:
        return None
    names = [e for e in o.listdir() if e[-1] in ('Workbook', 'Book')]
    if not names:
        return None
    d = o.openstream(names[0]).read()
    out = []
    for rid, body in recs(d):
        if rid == 0x1062 and len(body) >= 18:
            f = struct.unpack_from('<9H', body, 0)
            out.append(f)
    return out

roots = sys.argv[1:]
total_docs = 0
for root in roots:
    for dirpath, _, files in os.walk(root):
        for fn in sorted(files):
            p = os.path.join(dirpath, fn)
            got = scan(p)
            if not got:
                continue
            date = [f for f in got if f[8] & 0x0010]
            if date:
                total_docs += 1
                print(f'{os.path.relpath(p, root)}: {len(got)} CHDATERANGE, {len(date)} date-axis')
                for f in date:
                    print(f'    min={f[0]} max={f[1]} majStep={f[2]} majUnit={f[3]} '
                          f'minStep={f[4]} minUnit={f[5]} base={f[6]} cross={f[7]} flags=0x{f[8]:04X}')
print('documents with a date axis:', total_docs)
