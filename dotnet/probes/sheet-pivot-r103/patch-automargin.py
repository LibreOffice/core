#!/usr/bin/env python3
"""Set fAutoTextMargin on one shape of a BIFF workbook 26.2.4.2 wrote, in place.

    patch2.py <in.xls> <out.xls> <shape-index>

LibreOffice's own *MS Excel 97* filter never writes `DFF_Prop_AutoTextMargin` (188) --
`sc/source/filter/excel/` names it only where the *import* reads it -- so no round-tripped
fixture can carry the property. It does write property **191**, the text boolean group the
flag lives in, so the flag can be turned on by setting two bits of a value that is already
there: bit 3 of the low half (which is property 188) and bit 3 of the high "was stated"
half. Six bytes are rewritten and no length anywhere changes.
"""
import sys, struct, olefile

src, dst, index = sys.argv[1], sys.argv[2], int(sys.argv[3])
raw = bytearray(open(src, 'rb').read())
ole = olefile.OleFileIO(src)
name = [e for e in ole.listdir() if e[-1].lower() in ("workbook", "book")][0]
stream = ole.openstream(name).read()
origin = bytes(raw).find(stream)
if origin < 0:
    sys.exit("the workbook stream is not contiguous in the file")

pos, recs = 0, []
while pos + 4 <= len(stream):
    rid, rlen = struct.unpack_from("<HH", stream, pos)
    recs.append((pos, rid, rlen)); pos += 4 + rlen

MSO, CONT = 0x00EC, 0x003C
buf, spans, prev = bytearray(), [], None
for (p, rid, rlen) in recs:
    if rid == MSO or (rid == CONT and prev == MSO):
        spans.append((len(buf), len(buf) + rlen, p + 4)); buf += stream[p + 4:p + 4 + rlen]
        prev = MSO
    elif rid != CONT:
        prev = rid

def walk(b, s, e, d, out):
    p = s
    while p + 8 <= e:
        vi, t, ln = struct.unpack_from("<HHI", b, p)
        body, end = p + 8, min(p + 8 + ln, e)
        out.append([d, t, ln, p, body, end, vi >> 4])
        if (vi & 0xF) == 0xF: walk(b, body, end, d + 1, out)
        p = body + ln
    return out

tree = walk(bytes(buf), 0, len(buf), 0, [])
shape = [r for r in tree if r[1] == 0xF004][index]
opt = next(r for r in tree if r[1] == 0xF00B and shape[4] <= r[3] < shape[5])

at = None
p = opt[4]
for _ in range(opt[6]):
    pid, val = struct.unpack_from("<HI", buf, p)
    if (pid & 0x3FFF) == 191: at = p; break
    p += 6
if at is None:
    sys.exit("that shape's msofbtOPT states no property 191 to set the bit in")

pid, val = struct.unpack_from("<HI", buf, at)
new = val | (1 << 3) | (1 << 19)
struct.pack_into("<HI", buf, at, pid, new)
print(f"shape {index}: property 191 {val:#x} -> {new:#x}")

# back into the file, byte for byte
where = next(s for (s, e, o) in spans if s <= at < e)
off = next(o for (s, e, o) in spans if s <= at < e)
raw[origin + off + (at - where):origin + off + (at - where) + 6] = buf[at:at + 6]
open(dst, 'wb').write(bytes(raw))
