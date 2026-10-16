#!/usr/bin/env python3
"""Walk a BIFF workbook's record stream and report its chart substreams' font records.

A byte-level BOF-signature search does not work (a previous round measured zero on a
workbook that plainly draws charts); this does a real OLE2 + record-stream parse.
"""
import sys, struct, olefile

FONT = 0x0031
BOF = (0x0809, 0x0409, 0x0209, 0x0009)
EOF = 0x000A
CHCHART = 0x1002
CHFONT = 0x1026
CHTEXT = 0x1025
CHDEFAULTTEXT = 0x1024
CHBEGIN = 0x1033
CHEND = 0x1034
CHOBJECTLINK = 0x1027
CHFONTLIST = 0x1032
CHFRTFONTLIST = 0x089E


def records(data):
    at = 0
    n = len(data)
    while at + 4 <= n:
        rid, ln = struct.unpack_from('<HH', data, at)
        yield at, rid, data[at + 4:at + 4 + ln]
        at += 4 + ln


def read_font(payload, biff8):
    # FONT: height(2) flags(2) colour(2) weight(2) escapement(2) underline(1)
    #       family(1) charset(1) reserved(1) name
    if len(payload) < 15:
        return None
    height, flags, colour, weight = struct.unpack_from('<HHHH', payload, 0)
    ln = payload[14]
    if biff8:
        if len(payload) < 16:
            return None
        opt = payload[15]
        raw = payload[16:16 + ln * (2 if opt & 1 else 1)]
        name = raw.decode('utf-16-le' if opt & 1 else 'latin-1', 'replace')
    else:
        name = payload[15:15 + ln].decode('latin-1', 'replace')
    return name, height / 20.0, weight


def main(path):
    ole = olefile.OleFileIO(path)
    stream = None
    for nm in ('Workbook', 'Book'):
        if ole.exists(nm):
            stream = ole.openstream(nm).read()
            break
    if stream is None:
        print('no workbook stream')
        return
    fonts = []
    biff8 = False
    depth = 0
    in_chart = 0
    stack = []
    charts = 0
    pending_deftext = None
    findings = []
    for at, rid, payload in records(stream):
        if rid in BOF:
            if len(payload) >= 4:
                ver, typ = struct.unpack_from('<HH', payload, 0)
                biff8 = biff8 or ver >= 0x0600
                if typ == 0x0020:      # chart substream
                    in_chart = depth + 1
                    charts += 1
                    stack = []
            depth += 1
            continue
        if rid == EOF:
            depth -= 1
            if in_chart and depth < in_chart:
                in_chart = 0
            continue
        if rid == FONT and not in_chart:
            f = read_font(payload, biff8)
            # BIFF's phantom index 4
            if len(fonts) == 4:
                fonts.append(f)
            fonts.append(f)
            continue
        if in_chart and depth == in_chart:
            if rid == CHBEGIN:
                stack.append(header)
                continue
            if rid == CHEND:
                if stack:
                    stack.pop()
                continue
            header = rid
            if rid == CHDEFAULTTEXT:
                pending_deftext = struct.unpack_from('<H', payload, 0)[0]
            elif rid == CHFONT:
                idx = struct.unpack_from('<H', payload, 0)[0]
                fam = fonts[idx][0] if 0 <= idx < len(fonts) and fonts[idx] else '?'
                findings.append((charts, tuple(hex(s) for s in stack), idx, fam,
                                 pending_deftext))
    print(f'{path}')
    print(f'  BIFF8={biff8}  chart substreams={charts}  FONT records={len(fonts)}')
    for i, f in enumerate(fonts):
        if f:
            print(f'    font[{i}] {f[0]!r} {f[1]}pt w{f[2]}')
    print(f'  CHFONT records: {len(findings)}')
    seen = {}
    for c, st, idx, fam, dt in findings:
        key = (st, idx, fam, dt)
        seen[key] = seen.get(key, 0) + 1
    for (st, idx, fam, dt), n in sorted(seen.items(), key=lambda kv: -kv[1]):
        print(f'    x{n:<4} stack={st} fontIdx={idx} {fam!r} lastDefText={dt}')


if __name__ == '__main__':
    for p in sys.argv[1:]:
        main(p)
