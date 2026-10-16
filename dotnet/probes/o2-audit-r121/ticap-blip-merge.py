#!/usr/bin/env python3
"""Reproduce, outside X-Ray.Office, the transparent-bitmap merge of one TICAP blip.

The question round 94 left is a PATH question: do `TICAPCapability_Final.xls`'s paired
SRCAND blits reach a metafile reader at all?  This answers it by decoding the two DIBs
that make the pair straight out of the file, merging them the way
`RasterOperations.Merge` does (colour where the mask is black, transparent where it is
white), and comparing the result pixel for pixel against the image XObject and its
/SMask that our own renderer put on the page.

    ticap-blip-merge.py <xls> <blip-index> <colour.png> <smask.png>
"""
import struct, sys, zlib, pathlib
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from ole import read_ole
from esch import walk

SRCAND, SRCPAINT = 0x008800C6, 0x00EE0086
EMF_ROP = {76: 40, 77: 40, 81: 68}


def emf_records(d):
    pos = 0
    while pos + 8 <= len(d):
        typ, size = struct.unpack_from('<II', d, pos)
        if size < 8: return
        yield typ, pos, size
        pos += size


def dib_of(d, pos, typ):
    """The BITMAPINFO + bits of one blit record, as a standalone BMP."""
    if typ == 81:
        offBmi, cbBmi, offBits, cbBits = struct.unpack_from('<4I', d, pos + 48)
    else:
        offBmi, cbBmi, offBits, cbBits = struct.unpack_from('<4I', d, pos + 84)
    if cbBmi == 0 or cbBits == 0: return None
    bmi = d[pos + offBmi: pos + offBmi + cbBmi]
    bits = d[pos + offBits: pos + offBits + cbBits]
    return bmi, bits


def decode(bmi, bits):
    """A minimal bottom-up DIB decoder: 1 bpp and 24 bpp, which is all this file uses."""
    size, w, h, planes, bpp, comp = struct.unpack_from('<IiiHHI', bmi, 0)
    ncol = 0
    if bpp <= 8:
        ncol = 1 << bpp
    pal = [struct.unpack_from('<4B', bmi, size + 4 * i) for i in range(ncol)]
    stride = ((w * bpp + 31) // 32) * 4
    out = []
    for y in range(abs(h)):
        row = bits[(abs(h) - 1 - y) * stride:] if h > 0 else bits[y * stride:]
        line = []
        for x in range(w):
            if bpp == 1:
                bit = (row[x >> 3] >> (7 - (x & 7))) & 1
                b, g, r, _ = pal[bit]
            elif bpp == 24:
                b, g, r = row[3 * x], row[3 * x + 1], row[3 * x + 2]
            elif bpp == 32:
                b, g, r = row[4 * x], row[4 * x + 1], row[4 * x + 2]
            else:
                raise SystemExit('bpp %d not handled' % bpp)
            line.append((r, g, b))
        out.append(line)
    return w, abs(h), out


def main(path, want, colour_png, smask_png):
    entries, read = read_ole(path)
    s = read('Workbook')
    pos, group, cur = 0, bytearray(), None
    while pos + 4 <= len(s):
        rid, ln = struct.unpack_from('<HH', s, pos)
        b = s[pos + 4: pos + 4 + ln]
        if rid == 0x00EB: group += b; cur = 'g'
        elif rid == 0x003C and cur == 'g': group += b
        elif rid != 0x003C: cur = None
        pos += 4 + ln
    g = bytes(group)

    idx = 0
    for depth, off, rt, inst, ver, ln, body in walk(g):
        if rt != 0xF007: continue
        idx += 1
        if idx != want: continue
        vi2, rt2, ln2 = struct.unpack_from('<HHI', g, body + 36)
        inst2 = vi2 >> 4
        p = body + 44 + 16 * (2 if (inst2 & 1) else 1)
        data = g[p + 34: body + 44 + ln2]
        if g[p + 32] == 0: data = zlib.decompress(data)

        mask = image = None
        for typ, rpos, size in emf_records(data):
            if typ not in EMF_ROP: continue
            rop, = struct.unpack_from('<I', data, rpos + EMF_ROP[typ])
            got = dib_of(data, rpos, typ)
            if got is None: continue
            if rop == SRCAND: mask = decode(*got)
            elif rop == SRCPAINT: image = decode(*got)
        if mask is None or image is None:
            raise SystemExit('blip %d: no SRCAND/SRCPAINT pair with bitmaps' % want)

        mw, mh, mpx = mask
        iw, ih, ipx = image
        print('blip %d: mask %dx%d, image %dx%d' % (want, mw, mh, iw, ih))

        # RasterOperations.Merge: the mask's black keeps the colour, its white is knocked out.
        merged = [[(ipx[y][x], 0 if mpx[y][x] == (255, 255, 255) else 255)
                   for x in range(iw)] for y in range(ih)]

        from PIL import Image
        col = Image.open(colour_png).convert('RGB')
        sm = Image.open(smask_png).convert('L')
        bad_c = bad_a = 0
        for y in range(ih):
            for x in range(iw):
                if col.getpixel((x, y)) != merged[y][x][0]: bad_c += 1
                if sm.getpixel((x, y)) != merged[y][x][1]: bad_a += 1
        print('colour plane mismatches: %d of %d' % (bad_c, iw * ih))
        print('alpha  plane mismatches: %d of %d' % (bad_a, iw * ih))
        opaque = sum(1 for y in range(ih) for x in range(iw) if merged[y][x][1])
        print('opaque pixels in the merge: %d of %d' % (opaque, iw * ih))
        return
    raise SystemExit('blip %d not found' % want)


if __name__ == '__main__':
    main(sys.argv[1], int(sys.argv[2]), sys.argv[3], sys.argv[4])
