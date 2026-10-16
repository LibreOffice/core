#!/usr/bin/env python3
"""Census: which .xls charts state a manual outer plot area (CHFRAMEPOS on the primary axes set).

Walks each chart substream in the Workbook stream: CHCHART ... CHEND. Inside it, CHPROPERTIES'
flags give MANPLOTAREA/USEMANPLOTAREA, and the first CHAXESSET (id 0) group's CHFRAMEPOS is the
plot-area rectangle LibreOffice reads (xichart.cxx:4031-4048).
"""
import struct, sys, olefile, pathlib

CHCHART, CHBEGIN, CHEND = 0x1002, 0x1033, 0x1034
CHAXESSET, CHFRAMEPOS, CHPROPERTIES = 0x1041, 0x104F, 0x1044

def records(data):
    off = 0
    while off + 4 <= len(data):
        rid, rlen = struct.unpack_from('<HH', data, off)
        if off + 4 + rlen > len(data):
            return
        yield rid, data[off+4:off+4+rlen]
        off += 4 + rlen

def charts(path):
    try:
        ole = olefile.OleFileIO(path)
    except Exception as exc:
        return [('open-failed', str(exc))]
    names = [n for n in ole.listdir() if n[-1] in ('Workbook', 'Book')]
    if not names:
        return []
    data = ole.openstream(names[0]).read()
    out, depth, cur = [], 0, None
    for rid, p in records(data):
        if rid == CHCHART and cur is None and len(p) >= 16:
            x, y, w, h = struct.unpack_from('<iiii', p, 0)
            cur = {'size_pt': (w / 65536.0, h / 65536.0), 'flags': None,
                   'axesset': [], 'depth0': depth}
            out.append(cur)
        if rid == CHBEGIN:
            depth += 1
        elif rid == CHEND:
            depth -= 1
            if cur is not None and depth <= cur['depth0']:
                cur = None
        elif cur is not None:
            if rid == CHPROPERTIES and len(p) >= 2:
                cur['flags'] = struct.unpack_from('<H', p, 0)[0]
            elif rid == CHAXESSET and len(p) >= 2:
                cur['axesset'].append({'id': struct.unpack_from('<H', p, 0)[0],
                                       'depth': depth + 1, 'framepos': None})
            elif rid == CHFRAMEPOS and len(p) >= 20 and cur['axesset']:
                a = cur['axesset'][-1]
                if a['framepos'] is None and depth == a['depth']:
                    tl, br = struct.unpack_from('<HH', p, 0)
                    r = [struct.unpack_from('<h', p, 4 + i * 4)[0] for i in range(4)]
                    a['framepos'] = (tl, br, tuple(r))
    return out

def main(paths):
    print("path\tchart\tw_pt\th_pt\tflags\tmanplot\tuseman\tprimary_framepos")
    for path in paths:
        try:
            cs = charts(path)
        except Exception as exc:
            print(f"{path}\t-\t-\t-\tERROR {exc}")
            continue
        for i, c in enumerate(cs):
            if not isinstance(c, dict):
                print(f"{path}\t-\t-\t-\t{c}")
                continue
            f = c['flags']
            prim = next((a for a in c['axesset'] if a['id'] == 0), None)
            print(f"{path}\t{i}\t{c['size_pt'][0]:.2f}\t{c['size_pt'][1]:.2f}\t"
                  f"{'' if f is None else hex(f)}\t{'' if f is None else (f>>3)&1}\t"
                  f"{'' if f is None else (f>>4)&1}\t{prim['framepos'] if prim else None}")

main(sys.argv[1:])
