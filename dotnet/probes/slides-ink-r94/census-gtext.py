#!/usr/bin/env python3
"""Census: MS-binary shapes carrying WordArt, i.e. Escher property 192 (gtextUNICODE).

The property lives on an msofbtOPT (0xF00B) inside the drawing tree, and the drawing tree
lives in a different stream per host -- 'PowerPoint Document' for PPT, 'WordDocument'/'Data'
for DOC, 'Workbook' for XLS -- so every OLE2 stream is walked rather than a named one.
"""
import sys, struct, pathlib
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from ole import read_ole
from esch import walk

def gtexts(blob):
    out = []
    try:
        for depth, off, rt, inst, ver, ln, body in walk(blob):
            if rt != 0xF00B: continue
            props = []
            p = body
            ok = True
            for i in range(inst):
                if p + 6 > len(blob): ok = False; break
                pid, val = struct.unpack_from('<HI', blob, p); p += 6
                props.append((pid & 0x3FFF, bool(pid & 0x8000), val))
            if not ok: continue
            for pid, cflag, val in props:
                if not cflag: continue
                data = blob[p:p+val]; p += val
                if pid == 192 and 0 < val < 4000:
                    try: out.append(data.decode('utf-16-le', 'replace').rstrip('\x00'))
                    except Exception: pass
    except Exception:
        pass
    return out

rows = []
root = pathlib.Path(sys.argv[1])
for f in sorted(root.rglob('*')):
    if not f.is_file() or f.suffix.lower() not in ('.ppt', '.doc', '.xls'): continue
    try: entries, read = read_ole(str(f))
    except Exception: continue
    found = []
    for nm, t, st, sz in entries:
        if t != 2 or sz == 0: continue
        try: s = read(nm)
        except Exception: continue
        if s: found += gtexts(s)
    if found: rows.append((f.name, f.suffix.lower().lstrip('.'), len(found), found[0][:50]))
print('doc\text\tgtext_shapes\tfirst')
for r in rows: print('%s\t%s\t%d\t%s' % r)
print('# documents:', len(rows), ' # shapes:', sum(r[2] for r in rows))
