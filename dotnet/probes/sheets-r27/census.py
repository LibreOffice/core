#!/usr/bin/env python3
"""Census of BIFF chart substreams across the sheets track, by real record parse.

A BOF-signature byte search reports zero on a workbook that plainly draws charts, because
a chart substream's BOF is an ordinary 0x0809 record inside the Workbook stream and the
stream is stored in OLE2 sectors. This walks OLE2, then the record stream.
"""
import sys, os, struct, glob
import olefile

FONT = 0x0031
BOF = (0x0809, 0x0409, 0x0209, 0x0009)
EOF = 0x000A
CHFONT = 0x1026
CHDEFAULTTEXT = 0x1024
CHBEGIN = 0x1033
CHEND = 0x1034
CHTEXT = 0x1025
CHAXIS = 0x101D
CHLEGEND = 0x1015


def records(data):
    at, n = 0, len(data)
    while at + 4 <= n:
        rid, ln = struct.unpack_from('<HH', data, at)
        yield rid, data[at + 4:at + 4 + ln]
        at += 4 + ln


def read_font(payload, biff8):
    if len(payload) < 15:
        return None
    ln = payload[14]
    if biff8:
        if len(payload) < 16:
            return None
        opt = payload[15]
        raw = payload[16:16 + ln * (2 if opt & 1 else 1)]
        return raw.decode('utf-16-le' if opt & 1 else 'latin-1', 'replace')
    return payload[15:15 + ln].decode('latin-1', 'replace')


def scan(path):
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return None
    stream = None
    for nm in ('Workbook', 'Book'):
        if ole.exists(nm):
            stream = ole.openstream(nm).read()
            break
    if stream is None:
        return None

    fonts, biff8, depth, in_chart, charts = [], False, 0, 0, 0
    stack, header, pending = [], 0, None
    # per chart: {'global': fam, 'axesset': fam, 'first': fam, 'families': set}
    out = []
    cur = None
    for rid, payload in records(stream):
        if rid in BOF:
            if len(payload) >= 4:
                ver, typ = struct.unpack_from('<HH', payload, 0)
                biff8 = biff8 or ver >= 0x0600
                if typ == 0x0020 and not in_chart:
                    in_chart = depth + 1
                    charts += 1
                    stack, pending = [], None
                    cur = {'global': None, 'axesset': None, 'first': None,
                           'families': set(), 'n': 0}
            depth += 1
            continue
        if rid == EOF:
            depth -= 1
            if in_chart and depth < in_chart:
                in_chart = 0
                if cur:
                    out.append(cur)
                cur = None
            continue
        if rid == FONT and not in_chart:
            f = read_font(payload, biff8)
            if len(fonts) == 4:
                fonts.append(f)
            fonts.append(f)
            continue
        if in_chart and depth == in_chart and cur is not None:
            if rid == CHBEGIN:
                stack.append(header)
                continue
            if rid == CHEND:
                if stack:
                    stack.pop()
                continue
            header = rid
            if rid == CHDEFAULTTEXT:
                pending = struct.unpack_from('<H', payload, 0)[0]
            elif rid == CHFONT and len(payload) >= 2:
                idx = struct.unpack_from('<H', payload, 0)[0]
                fam = fonts[idx] if 0 <= idx < len(fonts) else None
                if not fam:
                    continue
                cur['n'] += 1
                cur['families'].add(fam)
                if cur['first'] is None:
                    cur['first'] = fam
                # a CHDEFAULTTEXT's CHTEXT is the group opened right after it, at depth 1
                if len(stack) == 2 and stack[-1] == CHTEXT and pending is not None:
                    if pending == 2 and cur['global'] is None:
                        cur['global'] = fam
                    if pending == 3 and cur['axesset'] is None:
                        cur['axesset'] = fam
    return out


def main(root):
    files = []
    for d in sorted(glob.glob(os.path.join(root, 'sheets', 'batch-*'))):
        for dirpath, _, names in os.walk(d):
            for n in names:
                files.append(os.path.join(dirpath, n))
    ole_n = chart_docs = chart_n = font_docs = 0
    disagree = []
    for p in sorted(files):
        r = scan(p)
        if r is None:
            continue
        ole_n += 1
        if not r:
            continue
        chart_docs += 1
        chart_n += len(r)
        if any(c['n'] for c in r):
            font_docs += 1
        fams = set()
        for c in r:
            fams |= c['families']
            g, a = c['global'], c['axesset']
            if g and a and g != a:
                disagree.append((p, 'global/axesset', g, a))
        print(f"{os.path.basename(p)}\tcharts={len(r)}\t"
              f"chfonts={sum(c['n'] for c in r)}\tfamilies={sorted(fams)}\t"
              f"global={[c['global'] for c in r][:3]}\taxesset={[c['axesset'] for c in r][:3]}")
    print(f"\n--- OLE2 workbooks scanned: {ole_n}")
    print(f"--- with >=1 chart substream: {chart_docs}  (total substreams {chart_n})")
    print(f"--- with >=1 resolvable CHFONT: {font_docs}")
    print(f"--- charts where global default font != axes-set default font: {len(disagree)}")
    for d in disagree[:10]:
        print('   ', d)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/workspace/sample-files')
