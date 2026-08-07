#!/usr/bin/env python3
"""Name the face LibreOffice chose for each probe row: facewidth2.py <families.txt> <pdf>"""
import re, struct, subprocess, sys

LETTERS = "Hamburgefonstiv"
DIGITS = "0123456789"
CANDIDATES = {
    'LiberationSans': '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf',
    'LiberationSerif': '/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf',
    'LiberationMono': '/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf',
    'DejaVuSans': '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',
    'DejaVuSerif': '/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf',
    'DejaVuSansMono': '/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf',
    'Carlito': '/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf',
    'Caladea': '/usr/share/fonts/truetype/crosextra/Caladea-Regular.ttf',
}


def tables(d):
    n = struct.unpack('>H', d[4:6])[0]
    return {d[12 + 16 * i:16 + 16 * i].decode('latin1'):
            struct.unpack('>II', d[20 + 16 * i:28 + 16 * i]) for i in range(n)}


def widths(path, texts, size):
    d = open(path, 'rb').read()
    t = tables(d)
    upem = struct.unpack('>H', d[t['head'][0] + 18:t['head'][0] + 20])[0]
    numh = struct.unpack('>H', d[t['hhea'][0] + 34:t['hhea'][0] + 36])[0]
    hmtx = t['hmtx'][0]
    co = t['cmap'][0]
    sub = None
    for i in range(struct.unpack('>H', d[co + 2:co + 4])[0]):
        pid, eid, off = struct.unpack('>HHI', d[co + 4 + 8 * i:co + 12 + 8 * i])
        if (pid, eid) in ((3, 1), (0, 3), (0, 4)) and \
                struct.unpack('>H', d[co + off:co + off + 2])[0] == 4:
            sub = co + off
    segx2 = struct.unpack('>H', d[sub + 6:sub + 8])[0]
    seg = segx2 // 2
    ends = struct.unpack(f'>{seg}H', d[sub + 14:sub + 14 + segx2])
    starts = struct.unpack(f'>{seg}H', d[sub + 16 + segx2:sub + 16 + 2 * segx2])
    deltas = struct.unpack(f'>{seg}h', d[sub + 16 + 2 * segx2:sub + 16 + 3 * segx2])
    rbase = sub + 16 + 3 * segx2
    ranges = struct.unpack(f'>{seg}H', d[rbase:rbase + segx2])

    def gid(c):
        for i in range(seg):
            if starts[i] <= c <= ends[i]:
                if ranges[i] == 0:
                    return (c + deltas[i]) & 0xFFFF
                a = rbase + 2 * i + ranges[i] + 2 * (c - starts[i])
                g = struct.unpack('>H', d[a:a + 2])[0]
                return (g + deltas[i]) & 0xFFFF if g else 0
        return 0

    return [sum(struct.unpack('>H', d[hmtx + 4 * min(gid(ord(c)), numh - 1):][:2])[0] for c in text)
            / upem * size for text in texts]


families = [l.rstrip('\n') for l in open(sys.argv[1], encoding='utf8') if l.strip()]
out = subprocess.run(['pdftotext', '-bbox', sys.argv[2], '-'], capture_output=True, text=True).stdout
pages = out.split('<page ')
byrow = {}
for page in pages[1:]:
    found = []
    for m in re.finditer(
            r'<word xMin="([0-9.]+)" yMin="([0-9.]+)" xMax="([0-9.]+)" yMax="([0-9.]+)">(.*?)</word>',
            page):
        x0, y0, x1, y1, w = (float(m.group(1)), float(m.group(2)), float(m.group(3)),
                             float(m.group(4)), m.group(5))
        found.append(((y0 + y1) / 2, w, x1 - x0))
    # Cluster by vertical centre: three cells of one row sit within a few points of each
    # other, and different faces put their baselines in different places.
    rows = {}
    for mid, w, width in sorted(found):
        key = next((k for k in rows if abs(k - mid) <= 4.0), None)
        if key is None:
            rows[mid] = {}
            key = mid
        rows[key][w] = width
    for y in sorted(rows):
        cell = rows[y]
        key = next((k for k in cell if re.fullmatch(r'R\d{4}', k)), None)
        if key and LETTERS in cell and DIGITS in cell:
            byrow[int(key[1:])] = (cell[LETTERS], cell[DIGITS])

exp = {name: widths(p, [LETTERS, DIGITS], 10.0) for name, p in CANDIDATES.items()}
if len(byrow) != len(families):
    missing = [i for i in range(len(families)) if i not in byrow]
    print(f"!! {len(byrow)} rows read for {len(families)} families; missing "
          + ', '.join(families[i] for i in missing), file=sys.stderr)
for i, family in enumerate(families):
    if i not in byrow:
        continue
    got = byrow[i]
    best = min(exp, key=lambda k: abs(exp[k][0] - got[0]) + abs(exp[k][1] - got[1]))
    err = abs(exp[best][0] - got[0]) + abs(exp[best][1] - got[1])
    print(f"{family}\t{best if err < 0.5 else 'UNKNOWN'}\t{got[0]:.2f}\t{got[1]:.2f}")
