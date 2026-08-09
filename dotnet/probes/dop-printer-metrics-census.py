#!/usr/bin/env python3
"""Census: which corpus .doc files set WW8Dop.fUsePrinterMetrics.

Reads the FIB's fcDop/lcbDop (rgfclcb index 31, i.e. byte 0x9A + 31*8 = 0x192),
pulls the Dop out of the table stream, and reports bit 31 of the dword at Dop+0x54.
Counts over: OLE2 .doc files only.  DOCX state it as <w:usePrinterMetrics/> in
word/settings.xml and none of the corpus does.
"""
import glob, os, struct, sys
import olefile

def dop_flags(path):
    if not olefile.isOleFile(path):
        return None
    ole = olefile.OleFileIO(path)
    try:
        if not ole.exists('WordDocument'):
            return None
        fib = ole.openstream('WordDocument').read(0x400)
        if len(fib) < 0x19A:
            return None
        flags = struct.unpack_from('<H', fib, 0x0A)[0]
        table = '1Table' if (flags & 0x0200) else '0Table'
        if not ole.exists(table):
            return None
        fcDop, lcbDop = struct.unpack_from('<ii', fib, 0x192)
        if lcbDop <= 0x58:
            return None
        tbl = ole.openstream(table).read()
        dop = tbl[fcDop:fcDop + lcbDop]
        if len(dop) < 0x58:
            return None
        opts = struct.unpack_from('<I', dop, 0x54)[0]
        return bool(opts & 0x80000000), lcbDop
    finally:
        ole.close()

root = sys.argv[1] if len(sys.argv) > 1 else '/workspace/sample-files/words'
n_ole = n_pm = 0
for p in sorted(glob.glob(os.path.join(root, '*/*/*')) + glob.glob(os.path.join(root, '*/*'))):
    if not os.path.isfile(p):
        continue
    try:
        r = dop_flags(p)
    except Exception as e:
        print('ERR', p, e); continue
    if r is None:
        continue
    n_ole += 1
    pm, lcb = r
    if pm:
        n_pm += 1
        print('printer-metrics', os.path.relpath(p, root), 'lcbDop=%d' % lcb)
print('OLE2 word documents read: %d; printer metrics: %d' % (n_ole, n_pm))
