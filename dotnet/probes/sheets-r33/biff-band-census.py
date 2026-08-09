#!/usr/bin/env python3
"""Census: which .xls sheets have a header/footer band Calc pins rather than makes dynamic,
and whose stated band is below Calc's 425-twip page-style default — the population the
BIFF band floor is wrong for."""
import struct, sys, os, re, glob
import olefile

def hf_nominal(codes, default_pt):
    """XclImpHFConverter::GetTotalHeight — sum of per-line max stated point sizes, max over
    the three portions. An empty portion still contributes one line."""
    parts = [[0.0], [0.0], [0.0]]      # per-portion list of line heights
    size = default_pt
    cur = [default_pt, default_pt, default_pt]
    part = 1
    i = 0
    sizes = [default_pt]*3
    lines = [[], [], []]
    line_max = [0.0, 0.0, 0.0]
    def bank(p):
        lines[p].append(line_max[p] if line_max[p] > 0 else sizes[p])
        line_max[p] = 0.0
    while i < len(codes):
        c = codes[i]
        if c != '&' or i+1 >= len(codes):
            if c == '\n': bank(part)
            else: line_max[part] = max(line_max[part], sizes[part])
            i += 1; continue
        code = codes[i+1]; i += 2
        if code in 'LCR':
            part = 'LCR'.index(code); sizes[part] = default_pt
        elif code in 'PNDTAFZ&':
            line_max[part] = max(line_max[part], sizes[part])
        elif code == '\n': bank(part)
        elif code == '"':
            e = codes.find('"', i); i = len(codes) if e < 0 else e+1
        elif code.isdigit():
            s = i-1
            while i < len(codes) and codes[i].isdigit(): i += 1
            sizes[part] = float(codes[s:i])
        elif code == 'K':
            t = 0
            while t < 6 and i < len(codes) and codes[i] in '0123456789abcdefABCDEF':
                i += 1; t += 1
    tot = 0.0
    for p in range(3):
        bank(p)
        tot = max(tot, sum(lines[p]))
    return tot           # points

def scan(path):
    try:
        f = olefile.OleFileIO(path)
    except Exception:
        return None
    stream = 'Workbook' if f.exists('Workbook') else ('Book' if f.exists('Book') else None)
    if stream is None: return None
    data = f.openstream(stream).read()
    i = 0; sheet = -1
    cur = {}
    out = []
    fontpt = None
    while i+4 <= len(data):
        rec, ln = struct.unpack('<HH', data[i:i+4]); body = data[i+4:i+4+ln]; i += 4+ln
        if rec == 0x0809:
            if cur: out.append(cur)
            sheet += 1; cur = {'sheet': sheet}
        elif rec == 0x0031 and fontpt is None and sheet == 0 and len(body) >= 2:
            fontpt = struct.unpack('<H', body[:2])[0] / 20.0
        elif rec in (0x0026, 0x0027, 0x0028, 0x0029) and len(body) >= 8:
            cur[{0x26:'l',0x27:'r',0x28:'t',0x29:'b'}[rec]] = struct.unpack('<d', body[:8])[0]
        elif rec == 0x00A1 and len(body) >= 32:
            hm, fm = struct.unpack('<dd', body[16:32]); cur['hm'] = hm; cur['fm'] = fm
        elif rec in (0x0014, 0x0015) and ln >= 3:
            cch = struct.unpack('<H', body[:2])[0]; fl = body[2]; raw = body[3:]
            try:
                s = raw[:cch*2].decode('utf-16le') if fl & 1 else raw[:cch].decode('latin1')
            except Exception:
                s = ''
            cur['hdr' if rec == 0x0014 else 'ftr'] = s
    if cur: out.append(cur)
    return out, (fontpt or 10.0)

hits = []
for path in sorted(glob.glob('/workspace/sample-files/sheets/*/*/*')):
    if os.path.isdir(path): continue
    r = scan(path)
    if not r: continue
    sheets, fontpt = r
    flagged = []
    for s in sheets:
        for kind, txt, mar, pag in (('hdr', s.get('hdr'), s.get('hm'), s.get('t')),
                                    ('ftr', s.get('ftr'), s.get('fm'), s.get('b'))):
            if not txt or mar is None or pag is None: continue
            stated = (pag - mar) * 1440.0
            nominal = hf_nominal(txt, fontpt) * 20.0
            if nominal > stated and stated < 425:
                flagged.append((s['sheet'], kind, round(stated,1), round(nominal,1)))
    if flagged:
        hits.append((path, flagged))
print(len(hits), 'documents flagged')
for p, f in hits:
    print(' ', os.path.basename(p), f[:4])
