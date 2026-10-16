#!/usr/bin/env python3
"""How many CONTINUE records each shape-path `TXO` uses, and whether r99's
"characters are in exactly one CONTINUE" assumption holds for it."""
import glob, os, struct, collections, olefile
CORPUS = "/home/user/sample-files"
TXO, CONTINUE, OBJ, FONT = 0x01B6, 0x003C, 0x005D, 0x0031
SHAPE_TYPES = {6, 7}
def workbook(p):
    try: ole = olefile.OleFileIO(p)
    except Exception: return None
    for n in ("Workbook", "Book"):
        if ole.exists(n): return ole.openstream(n).read()
    return None
def records(d):
    at = 0
    while at + 4 <= len(d):
        rid, size = struct.unpack_from("<HH", d, at)
        b = at + 4
        if b + size > len(d): return
        yield rid, d[b:b+size]
        at = b + size
tot = collections.Counter()
per = collections.defaultdict(collections.Counter)
for path in sorted(glob.glob(os.path.join(CORPUS, "**", "*.[xX][lL][sS]"), recursive=True)):
    data = workbook(path)
    if data is None: continue
    name = os.path.basename(path)
    objtype = None; pend = None; sizes = []
    def flush():
        global pend, sizes
        if pend is None: return
        chars, fsize, ot = pend
        if chars > 0 and ot in SHAPE_TYPES:
            tot['boxes'] += 1
            tot['continues=%d' % min(len(sizes), 9)] += 1
            # r99 read the SECOND continue as the run array; correct is the tail of all of them
            if len(sizes) > 2: tot['boxes needing >1 char CONTINUE'] += 1; per[name]['multi'] += 1
        pend = None; sizes = []
    for rid, body in records(data):
        if rid == FONT: continue
        if rid == OBJ and len(body) >= 10:
            ft = struct.unpack_from("<H", body, 0)[0]
            objtype = struct.unpack_from("<H", body, 4)[0] if ft == 0x15 else None
            continue
        if rid == TXO and len(body) >= 14:
            flush()
            chars, fsize = struct.unpack_from("<HH", body, 10)
            pend = (chars, fsize, objtype); sizes = []
            continue
        if rid == CONTINUE and pend is not None:
            sizes.append(len(body)); continue
        flush()
    flush()
for k in sorted(tot): print('%-34s %d' % (k, tot[k]))
print('--- per document (boxes whose characters span >1 CONTINUE)')
for d, c in sorted(per.items()): print('   %-56s %d' % (d, c['multi']))
