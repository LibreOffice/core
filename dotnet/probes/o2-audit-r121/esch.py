import struct
CONTAINERS = set()
def walk(buf, off=0, end=None, depth=0, out=None):
    if end is None: end = len(buf)
    while off + 8 <= end:
        ver_inst, rt, ln = struct.unpack_from('<HHI', buf, off)
        ver = ver_inst & 0xF; inst = ver_inst >> 4
        body = off+8
        yield (depth, off, rt, inst, ver, ln, body)
        if ver == 0xF:
            yield from walk(buf, body, min(end, body+ln), depth+1)
        off = body + ln
