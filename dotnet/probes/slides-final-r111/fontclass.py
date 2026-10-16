#!/usr/bin/env python3
"""Census, over a .ppt column, of faces whose family CLASS changes what fontconfig answers.

26.2.4.2 appends "serif" for FAMILY_ROMAN and "sans" for FAMILY_SWISS when it MEASURES a run
(`FontConfigManager::Substitute`, vcl/unx/generic/font/fontconfig.cxx:1075-1088), and the draw
layer rebuilds the font at FAMILY_DONTKNOW and therefore WITHOUT the generic
(`getVclFontFromFontAttribute`, drawinglayer/.../textlayoutdevice.cxx:416-448).  A face where the
two patterns resolve differently is measured in one file and drawn from another.
"""
import struct, subprocess, sys, pathlib, olefile

FAMILY = {1: 'serif', 2: 'sans', 3: 'monospace', 4: 'cursive', 5: 'fantasy'}

def recs(buf, off, end, d=0):
    while off + 8 <= end:
        vi, rt, rl = struct.unpack_from('<HHI', buf, off)
        ver = vi & 0xF
        body, stop = off + 8, min(off + 8 + rl, end)
        if rt == 4023:
            yield body, stop
        if ver == 0xF and d < 10:
            yield from recs(buf, body, stop, d + 1)
        off = body + rl

def match(pattern):
    return subprocess.run(['fc-match', pattern], capture_output=True, text=True).stdout.split(':')[0]

cache = {}
def answer(p):
    if p not in cache: cache[p] = match(p)
    return cache[p]

docs = 0
hits = {}
for f in sorted(pathlib.Path(sys.argv[1]).rglob('*')):
    if f.suffix.lower() not in ('.ppt', '.pot', '.pps'): continue
    docs += 1
    try:
        buf = olefile.OleFileIO(str(f)).openstream('PowerPoint Document').read()
    except Exception:
        continue
    for body, stop in recs(buf, 0, len(buf)):
        raw = buf[body:body + 64]
        name = raw.decode('utf-16-le', 'ignore').split('\0')[0].strip()
        if not name or body + 68 > stop: continue
        generic = FAMILY.get(buf[body + 67] >> 4)
        if generic is None: continue
        drawn, measured = answer(name), answer(f'{name},{generic}')
        if drawn != measured:
            hits.setdefault((name, generic, drawn, measured), set()).add(f.name)

print(f'documents {docs}')
total = set()
for (name, generic, drawn, measured), files in sorted(hits.items(), key=lambda kv: -len(kv[1])):
    total |= files
    print(f'  {name!r} ({generic}): draws {drawn}, measures {measured} -- {len(files)} document(s)')
print(f'documents naming at least one such face: {len(total)}')
