#!/usr/bin/env python3
"""Rewrite one `InteractiveInfoAtom`'s `exHyperlinkId` in a copy of a `.ppt`."""
import struct, sys, shutil
import olefile, census
from persist import directory, current_user_edit
TX, II, IIA = 4063, 4082, 4083

def spots(buf, roots):
    out = []
    def scan(s, e):
        seq = list(census.records(buf, s, e))
        for i, (ver, rt, b, st) in enumerate(seq):
            if rt == II:
                nxt = seq[i + 1] if i + 1 < len(seq) else None
                if nxt and nxt[1] == TX:
                    for v2, t2, b2, s2 in census.records(buf, b, st):
                        if t2 == IIA: out.append(b2 + 4)
            elif ver == 0x0F: scan(b, st)
    for rt, off, end in roots: scan(off + 8, end)
    return out

src, dst, index, newid = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
ole = olefile.OleFileIO(src)
buf = bytearray(ole.openstream('PowerPoint Document').read())
cur = current_user_edit(ole); ole.close()
offs, doc = directory(bytes(buf), cur)
roots = census.live_roots(bytes(buf), offs, doc)
s = spots(bytes(buf), roots)
print(f'patching #{index} of {len(s)} at {s[index]}: '
      f'{struct.unpack_from("<I", buf, s[index])[0]} -> {newid}', file=sys.stderr)
struct.pack_into('<I', buf, s[index], newid)
shutil.copyfile(src, dst)
with olefile.OleFileIO(dst, write_mode=True) as out:
    out.write_stream('PowerPoint Document', bytes(buf))
