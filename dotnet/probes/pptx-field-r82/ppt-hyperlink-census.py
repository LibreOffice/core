"""Count .ppt carrying a text-range hyperlink, by scanning for the record headers.

PPT_PST_TxInteractiveInfoAtom (4063) is the atom that names a *text range*; it is the one
svdfppt.cxx:6917-6936 needs before it builds an SvxURLField.  A shape-level click action
carries a PPT_PST_InteractiveInfo (4082) with no TxInteractiveInfoAtom beside it, exactly as
a p:cNvPr/a:hlinkClick does in DrawingML.
"""
import struct, sys
from pathlib import Path
ROOT = Path('/home/user/sample-files')
def count(path, rectype):
    data = path.read_bytes()
    n = 0
    # a record header is <u16 verInst><u16 recType><u32 len>; scan every even offset
    tgt = struct.pack('<H', rectype)
    i = 2
    while True:
        i = data.find(tgt, i)
        if i < 0: break
        if i % 2 == 0:
            ln = struct.unpack_from('<I', data, i + 2)[0] if i + 6 <= len(data) else 1 << 40
            if ln < len(data):
                n += 1
        i += 2
    return n
tot = docs = 0
shape = 0
for p in sorted(ROOT.rglob('*.ppt')):
    try:
        a = count(p, 4063)
        b = count(p, 4082)
    except Exception:
        continue
    if a: docs += 1; tot += a
    if b: shape += 1
print(f'.ppt with a TxInteractiveInfoAtom (text-range hyperlink): {docs} documents, {tot} atoms')
print(f'.ppt with any InteractiveInfo at all: {shape} documents')
