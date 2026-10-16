#!/usr/bin/env python3
"""Font size and baseline y of every text show in a PDF, straight out of the
content stream's own Tf / Td / TD / Tm / T* / TL operators.  No bounding boxes:
pdftotext -bbox reports an ink box derived from the font descriptor, and
PyMuPDF rawdict reports a character cell, and both have produced a retracted
finding on this seat."""
import re, sys, zlib, collections

def objects(data):
    objs = {}
    for m in re.finditer(rb'(\d+)\s+(\d+)\s+obj\b', data):
        objs[int(m.group(1))] = m.end()
    return objs

def stream_at(data, pos):
    m = re.compile(rb'stream\r?\n').search(data, pos)
    if not m: return None, data[pos:pos+2000]
    dic = data[pos:m.start()]
    end = data.find(b'endstream', m.end())
    raw = data[m.end():end]
    if b'FlateDecode' in dic:
        try: raw = zlib.decompress(raw)
        except Exception: return dic, b''
    return dic, raw

def page_streams(path):
    data = open(path,'rb').read()
    objs = objects(data)
    # page objects in file order
    pages = []
    for num, pos in sorted(objs.items()):
        head = data[pos:pos+700]
        if re.search(rb'/Type\s*/Page\b', head):
            cm = re.search(rb'/Contents\s+(\d+)\s+\d+\s+R', head)
            res = re.search(rb'/Resources\s+(\d+)\s+\d+\s+R', head)
            if cm:
                pages.append((num, int(cm.group(1)), int(res.group(1)) if res else None, pos))
    out = []
    for num, cnum, rnum, pos in pages:
        _, body = stream_at(data, objs[cnum])
        fonts = {}
        rdic = data[objs[rnum]:objs[rnum]+8000] if (rnum and rnum in objs) else data[pos:pos+8000]
        fm = re.search(rb'/Font\s*(\d+)\s+\d+\s+R', rdic)
        if fm and int(fm.group(1)) in objs:
            o2 = objs[int(fm.group(1))]
            fdic = data[o2:o2+8000]
        else:
            fm2 = re.search(rb'/Font\s*<<', rdic)
            fdic = rdic[fm2.start():fm2.start()+8000] if fm2 else rdic
        for em in re.finditer(rb'/([A-Za-z0-9_.+-]+)\s+(\d+)\s+\d+\s+R', fdic):
            fobj = int(em.group(2))
            if fobj in objs:
                fh = data[objs[fobj]:objs[fobj]+800]
                bn = re.search(rb'/BaseFont\s*/([^\s/>\]]+)', fh)
                if bn: fonts[em.group(1).decode()] = bn.group(1).decode()
        out.append((body, fonts))
    return out

NUM = r'[-+]?[\d.]+'
def shows(body, fonts):
    text = body.decode('latin1')
    size = 0.0; font = ''
    a=d=1.0; b=c=0.0; e=f=0.0
    tlm = None; leading = 0.0
    res = []
    tok = re.compile(
        r'/(\S+)\s+(%s)\s+Tf|(%s)\s+(%s)\s+(%s)\s+(%s)\s+(%s)\s+(%s)\s+Tm|'
        r'(%s)\s+(%s)\s+(TD|Td)|(%s)\s+TL|(T\*)|BT|ET|'
        r'(?:\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>)\s*(?:TJ|Tj)|\]\s*TJ'
        % (NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM))
    for m in tok.finditer(text):
        s = m.group(0)
        if s.endswith('Tf'):
            font = m.group(1); size = float(m.group(2))
        elif s.endswith('Tm'):
            a,b,c,d,e,f = (float(m.group(i)) for i in range(3,9))
            tlm = (a,b,c,d,e,f)
        elif s.endswith('Td') or s.endswith('TD'):
            tx,ty = float(m.group(9)), float(m.group(10))
            if s.endswith('TD'): leading = -ty
            if tlm:
                a2,b2,c2,d2,e2,f2 = tlm
                tlm = (a2,b2,c2,d2, e2 + tx*a2 + ty*c2, f2 + tx*b2 + ty*d2)
        elif s.endswith('TL'):
            leading = float(m.group(12))
        elif s.startswith('T*'):
            if tlm:
                a2,b2,c2,d2,e2,f2 = tlm
                tlm = (a2,b2,c2,d2, e2 - leading*c2, f2 - leading*d2)
        elif s == 'BT':
            tlm = (1,0,0,1,0,0)
        elif s.endswith('TJ') or s.endswith('Tj'):
            if tlm:
                a2,b2,c2,d2,e2,f2 = tlm
                res.append((fonts.get(font, font), round(size*d2, 4), round(f2, 4)))
    return res

if __name__ == '__main__':
    for path in sys.argv[1:]:
        for i,(body,fonts) in enumerate(page_streams(path), 1):
            for fn, sz, y in shows(body, fonts):
                print(f"{path}\t{i}\t{fn}\t{sz}\t{y}")
