#!/usr/bin/env python3
"""What CHAREAFORMAT/CHLINEFORMAT the corpus's BIFF chart substreams actually state."""
import sys, struct, glob, os
import olefile

BOF = (0x0809, 0x0409, 0x0209, 0x0009)
EOF = 0x000A
PALETTE = 0x0092
CHLINEFORMAT = 0x1007
CHAREAFORMAT = 0x100A
CHBEGIN = 0x1033
CHEND = 0x1034

NAMES = {0x1002: 'CHCHART', 0x1003: 'CHSERIES', 0x1006: 'CHDATAFORMAT',
         0x1014: 'CHTYPEGROUP', 0x1015: 'CHLEGEND', 0x101D: 'CHAXIS',
         0x1025: 'CHTEXT', 0x1032: 'CHFRAME', 0x1041: 'CHAXESSET',
         0x1021: 'CHAXISLINE', 0x1024: 'CHDEFAULTTEXT'}


def records(data):
    at, n = 0, len(data)
    while at + 4 <= n:
        rid, ln = struct.unpack_from('<HH', data, at)
        yield rid, data[at + 4:at + 4 + ln]
        at += 4 + ln


def scan(path):
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return
    stream = None
    for nm in ('Workbook', 'Book'):
        if ole.exists(nm):
            stream = ole.openstream(nm).read()
            break
    if stream is None:
        return
    depth = in_chart = 0
    stack, header = [], 0
    area = {'auto': 0, 'stated': 0}
    line = {'auto': 0, 'stated': 0}
    where = {}
    for rid, payload in records(stream):
        if rid in BOF:
            if len(payload) >= 4 and struct.unpack_from('<HH', payload, 0)[1] == 0x0020 \
                    and not in_chart:
                in_chart = depth + 1
                stack = []
            depth += 1
            continue
        if rid == EOF:
            depth -= 1
            if in_chart and depth < in_chart:
                in_chart = 0
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
            ctx = '/'.join(NAMES.get(s, hex(s)) for s in stack)
            if rid == CHAREAFORMAT and len(payload) >= 12:
                flags = struct.unpack_from('<H', payload, 10)[0]
                k = 'auto' if flags & 1 else 'stated'
                area[k] += 1
                where.setdefault(('area', k, ctx), 0)
                where[('area', k, ctx)] += 1
            elif rid == CHLINEFORMAT and len(payload) >= 10:
                flags = struct.unpack_from('<H', payload, 8)[0]
                k = 'auto' if flags & 1 else 'stated'
                line[k] += 1
                where.setdefault(('line', k, ctx), 0)
                where[('line', k, ctx)] += 1
    if area['auto'] + area['stated'] + line['auto'] + line['stated'] == 0:
        return
    print(f'{os.path.basename(path)}')
    print(f'   CHAREAFORMAT auto={area["auto"]} stated={area["stated"]}   '
          f'CHLINEFORMAT auto={line["auto"]} stated={line["stated"]}')
    for (kind, k, ctx), n in sorted(where.items(), key=lambda kv: -kv[1])[:8]:
        print(f'     {kind:5} {k:6} x{n:<4} in {ctx}')


for d in sorted(glob.glob('/workspace/sample-files/sheets/batch-*')):
    for dirpath, _, names in os.walk(d):
        for n in sorted(names):
            scan(os.path.join(dirpath, n))
