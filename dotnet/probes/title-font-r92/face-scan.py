#!/usr/bin/env python3
"""Score every font face on the box against a set of target advances, expressed
in thousandths of an em (the unit a PDF /Widths array uses).

usage: face-scan.py <targets.json> [extra font dirs...]
targets.json: [{"ch": "C", "thou": 733.0}, ...]
"""
import sys, os, json, struct, glob

def read_face(path, index=0):
    with open(path, "rb") as fh:
        data = fh.read()
    off = 0
    tag = data[:4]
    if tag == b"ttcf":
        n = struct.unpack(">I", data[8:12])[0]
        offs = struct.unpack(">%dI" % n, data[12:12+4*n])
        off = offs[index]
    numTables = struct.unpack(">H", data[off+4:off+6])[0]
    tabs = {}
    for i in range(numTables):
        p = off + 12 + 16*i
        t = data[p:p+4]
        o, l = struct.unpack(">II", data[p+8:p+16])
        tabs[t] = (o, l)
    if b"head" not in tabs or b"hmtx" not in tabs or b"cmap" not in tabs:
        return None
    ho, _ = tabs[b"head"]
    upem = struct.unpack(">H", data[ho+18:ho+20])[0]
    io = tabs[b"hhea"][0]
    numH = struct.unpack(">H", data[io+34:io+36])[0]
    mo, _ = tabs[b"hmtx"]
    def adv(gid):
        if gid < numH:
            return struct.unpack(">H", data[mo+4*gid:mo+4*gid+2])[0]
        return struct.unpack(">H", data[mo+4*(numH-1):mo+4*(numH-1)+2])[0]
    # cmap: prefer 3,10 then 3,1 then 0,x
    co, _ = tabs[b"cmap"]
    n = struct.unpack(">H", data[co+2:co+4])[0]
    best = None
    for i in range(n):
        pid, eid, so = struct.unpack(">HHI", data[co+4+8*i:co+12+8*i])
        rank = {(3,10):0, (3,1):1, (0,4):2, (0,3):3, (0,6):2}.get((pid,eid), 5)
        if best is None or rank < best[0]:
            best = (rank, co+so)
    sub = best[1]
    fmt = struct.unpack(">H", data[sub:sub+2])[0]
    cmap = {}
    if fmt == 4:
        segX2 = struct.unpack(">H", data[sub+6:sub+8])[0]
        seg = segX2 // 2
        ends   = struct.unpack(">%dH" % seg, data[sub+14:sub+14+segX2])
        starts = struct.unpack(">%dH" % seg, data[sub+16+segX2:sub+16+2*segX2])
        deltas = struct.unpack(">%dh" % seg, data[sub+16+2*segX2:sub+16+3*segX2])
        rngOff = sub+16+3*segX2
        rngs   = struct.unpack(">%dH" % seg, data[rngOff:rngOff+segX2])
        for i in range(seg):
            for c in range(starts[i], min(ends[i], 0xFFFF)+1):
                if rngs[i] == 0:
                    g = (c + deltas[i]) & 0xFFFF
                else:
                    p = rngOff + 2*i + rngs[i] + 2*(c - starts[i])
                    if p+2 > len(data): continue
                    g = struct.unpack(">H", data[p:p+2])[0]
                    if g: g = (g + deltas[i]) & 0xFFFF
                if g: cmap[c] = g
    elif fmt == 12:
        ngroups = struct.unpack(">I", data[sub+12:sub+16])[0]
        for i in range(ngroups):
            s, e, g = struct.unpack(">III", data[sub+16+12*i:sub+28+12*i])
            for c in range(s, min(e, s+0x10000)+1):
                cmap[c] = g + c - s
    else:
        return None
    # name
    name = os.path.basename(path)
    if b"name" in tabs:
        no, _ = tabs[b"name"]
        cnt, so = struct.unpack(">HH", data[no+2:no+6])
        for i in range(cnt):
            pid, eid, lid, nid, ln, o = struct.unpack(">HHHHHH", data[no+6+12*i:no+18+12*i])
            if nid == 6:
                raw = data[no+so+o:no+so+o+ln]
                try:
                    name = raw.decode("utf-16-be") if pid == 3 else raw.decode("latin-1")
                except Exception:
                    pass
                break
    return name, upem, cmap, adv

targets = json.load(open(sys.argv[1]))
dirs = sys.argv[2:] or ["/usr/share/fonts", "/opt/libreoffice26.2/share/fonts"]
files = []
for d in dirs:
    for ext in ("ttf", "otf", "ttc", "TTF", "OTF"):
        files += glob.glob(os.path.join(d, "**", "*." + ext), recursive=True)
files = sorted(set(files))

rows = []
for f in files:
    try:
        r = read_face(f)
    except Exception:
        r = None
    if not r: continue
    name, upem, cmap, adv = r
    err = 0.0; miss = 0; per = []
    for t in targets:
        cp = ord(t["ch"])
        g = cmap.get(cp)
        if not g:
            miss += 1; per.append(None); continue
        thou = adv(g) * 1000.0 / upem
        per.append(thou)
        err += abs(thou - t["thou"])
    if miss: continue
    rows.append((err, name, f, per))
rows.sort()
print(f"{'sum|err| (thou)':>15}  {'face':40s} file")
for err, name, f, per in rows[:12]:
    print(f"{err:15.1f}  {name:40s} {f}")
print()
best = rows[0]
print("per-glyph, best face:", best[1])
print(f"{'ch':>3} {'target':>8} {'face':>8} {'diff':>7}")
for t, v in zip(targets, best[3]):
    print(f"{t['ch']!r:>3} {t['thou']:8.1f} {v:8.1f} {v-t['thou']:7.1f}")
