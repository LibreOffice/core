#!/usr/bin/env python3
"""Rewrite one `TxInteractiveInfoAtom`'s character range in a copy of a `.ppt`.

A one-attribute variant, the project's standard instrument: everything else about the file
is byte-identical, so any difference in 26.2.4.2's rendering is that attribute's.
"""
import struct, sys, shutil, pathlib
import olefile, census
from persist import directory, current_user_edit

TX, II = 4063, 4082

def tx_offsets(buf, roots):
    out = []
    def scan(s, e):
        seq = list(census.records(buf, s, e))
        for i, (ver, rt, b, st) in enumerate(seq):
            if rt == II:
                nxt = seq[i + 1] if i + 1 < len(seq) else None
                if nxt and nxt[1] == TX and nxt[3] - nxt[2] >= 8:
                    out.append(nxt[2])
            elif ver == 0x0F:
                scan(b, st)
    for rt, off, end in roots: scan(off + 8, end)
    return out

def main(src, dst, index, start, end):
    ole = olefile.OleFileIO(str(src))
    buf = bytearray(ole.openstream('PowerPoint Document').read())
    cur = current_user_edit(ole); ole.close()
    offs, doc = directory(bytes(buf), cur)
    roots = census.live_roots(bytes(buf), offs, doc)
    spots = tx_offsets(bytes(buf), roots)
    print(f'{len(spots)} text ranges; patching #{index} at {spots[index]}', file=sys.stderr)
    struct.pack_into('<II', buf, spots[index], start, end)
    shutil.copyfile(src, dst)
    with olefile.OleFileIO(dst, write_mode=True) as out:
        out.write_stream('PowerPoint Document', bytes(buf))

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5]))
