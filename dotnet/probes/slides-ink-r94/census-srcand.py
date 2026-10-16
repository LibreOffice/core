#!/usr/bin/env python3
"""Census: metafiles in the corpus that blit with a LONE SRCAND raster operation.

A WMF/EMF says "transparent bitmap" either as a PAIR (a monochrome mask with SRCAND
followed by the colour image with SRCPAINT/SRCINVERT at the same rectangle) or, when the
image IS its own mask, as a SINGLE SRCAND blit.  LibreOffice resolves both:
mtftools.cxx:2626-2652 for the pair, and case 0x7/0x8 of the single-blit switch at
:2691-2708, which builds `Bitmap aBmpEx(aBitmap, aMask)` with the bitmap as its own mask.

This counts the second kind, which this tree drew opaquely.

Metafiles are reached three ways: zip parts for OOXML, Escher BLIP records inside every
OLE2 stream for the MS binaries (the PPT `Pictures` stream and the DOC/XLS drawing group
use the same record shape), and the raw file itself.
"""
import struct, sys, zlib, zipfile, pathlib, collections, io
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from ole import read_ole

SRCCOPY, SRCPAINT, SRCAND, SRCINVERT = 0x00CC0020, 0x00EE0086, 0x008800C6, 0x00660046
WMF_BLITS = {0x0940: 'DIBBITBLT', 0x0B41: 'DIBSTRETCHBLT', 0x0F43: 'STRETCHDIB',
             0x0922: 'BITBLT', 0x0B23: 'STRETCHBLT', 0x0D33: 'SETDIBTODEV'}

def wmf_blits(d):
    """Yield (rop, destrect) for each blit record of a WMF starting at its header."""
    if len(d) < 18: return
    pos = 18
    n = 0
    while pos + 6 <= len(d) and n < 200000:
        size, fn = struct.unpack_from('<IH', d, pos)
        if size < 3: return
        if fn in WMF_BLITS:
            try:
                rop, = struct.unpack_from('<I', d, pos + 6)
                if fn in (0x0940, 0x0922):        # BitBlt: no src extent
                    ys, xs, dh, dw, yd, xd = struct.unpack_from('<6h', d, pos + 10)
                elif fn == 0x0F43:                 # StretchDIBits
                    usage, sh, sw, ys, xs, dh, dw, yd, xd = struct.unpack_from('<9h', d, pos + 10)
                elif fn == 0x0D33:
                    yield (rop, None); pos += size * 2; n += 1; continue
                else:
                    sh, sw, ys, xs, dh, dw, yd, xd = struct.unpack_from('<8h', d, pos + 10)
                yield (rop, (xd, yd, dw, dh))
            except struct.error:
                return
        pos += size * 2
        n += 1

# The raster operation sits at a DIFFERENT offset in EMR_STRETCHDIBITS than in EMR_BITBLT,
# and reading the BitBlt offset for all three is what made this census miss four of the seven
# documents a rendered sweep found.  EMR_BITBLT/EMR_STRETCHBLT: Bounds(16) xDest yDest cxDest
# cyDest then the rop, at +40.  EMR_STRETCHDIBITS: Bounds(16) xDest yDest xSrc ySrc cxSrc cySrc
# offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc UsageSrc then the rop, at +68.
EMF_BLIT_ROP_OFFSET = {76: 40, 77: 40, 81: 68}


def emf_blits(d):
    if len(d) < 88: return
    pos = 0
    n = 0
    while pos + 8 <= len(d) and n < 200000:
        typ, size = struct.unpack_from('<II', d, pos)
        if size < 8: return
        if typ in EMF_BLIT_ROP_OFFSET:
            try:
                xd, yd = struct.unpack_from('<2i', d, pos + 24)
                rop, = struct.unpack_from('<I', d, pos + EMF_BLIT_ROP_OFFSET[typ])
                if typ == 81:
                    cxd, cyd = struct.unpack_from('<2i', d, pos + 72)
                else:
                    cxd, cyd = struct.unpack_from('<2i', d, pos + 32)
                yield (rop, (xd, yd, cxd, cyd))
            except struct.error:
                return
        pos += size
        n += 1

def classify(blits):
    """Return (n_lone_srcand, n_paired)."""
    lone = paired = 0
    i = 0
    b = list(blits)
    while i < len(b):
        rop, rect = b[i]
        if rop == SRCAND:
            if i + 1 < len(b) and b[i + 1][0] in (SRCPAINT, SRCINVERT) and b[i + 1][1] == rect:
                paired += 1; i += 2; continue
            if i and b[i - 1][0] == SRCPAINT and b[i - 1][1] == rect:
                i += 1; continue
            lone += 1
        elif rop == SRCPAINT and i + 1 < len(b) and b[i + 1][0] == SRCAND and b[i + 1][1] == rect:
            paired += 1; i += 2; continue
        i += 1
    return lone, paired

def metafiles(blob):
    """Yield metafile bytes found as Escher BLIPs inside one blob."""
    pos = 0
    n = 0
    while True:
        # scan for a BLIP record header
        found = -1
        for rt in (0xF01A, 0xF01B, 0xF01C, 0xF01D, 0xF01E, 0xF01F):
            at = blob.find(struct.pack('<H', rt), pos)
            while at >= 2:
                ver_inst, = struct.unpack_from('<H', blob, at - 2)
                if (ver_inst & 0xF) == 0 and at + 6 <= len(blob):
                    ln, = struct.unpack_from('<I', blob, at + 2)
                    if 40 < ln < len(blob) - at:
                        if found < 0 or at < found:
                            found = at - 2
                        break
                at = blob.find(struct.pack('<H', rt), at + 1)
        if found < 0: return
        ver_inst, rt, ln = struct.unpack_from('<HHI', blob, found)
        inst = ver_inst >> 4
        body = found + 8
        nuid = 2 if inst in (0x217, 0x3D5, 0x46B, 0x6E1, 0x7A9, 0x6E3, 0x543) else 1
        p = body + 16 * nuid
        if rt in (0xF01A, 0xF01B, 0xF01C) and p + 34 <= len(blob):
            comp = blob[p + 32]
            data = blob[p + 34: body + ln]
            if comp == 0:
                try: data = zlib.decompress(data)
                except Exception: data = b''
            if data: yield rt, data
        pos = body + max(ln, 1)
        n += 1
        if n > 5000: return

def scan(path):
    """Return (lone, paired) summed over every metafile reachable in the document."""
    lone = paired = 0
    blobs = []
    d = path.read_bytes()
    if d[:2] == b'PK':
        try:
            with zipfile.ZipFile(io.BytesIO(d)) as z:
                for nm in z.namelist():
                    if nm.lower().endswith(('.wmf', '.emf')):
                        try: blobs.append(('raw', z.read(nm)))
                        except Exception: pass
        except Exception: pass
    elif d[:8] == b'\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1':
        try:
            entries, read = read_ole(str(path))
            for nm, t, st, sz in entries:
                if t != 2 or sz == 0: continue
                try: s = read(nm)
                except Exception: continue
                if not s: continue
                for rt, mf in metafiles(s):
                    blobs.append(('emf' if rt == 0xF01A else 'wmf', mf))
        except Exception: pass
    for kind, b in blobs:
        if kind == 'raw':
            if b[:4] == b'\x01\x00\x09\x00' or b[:4] == b'\xd7\xcd\xc6\x9a':
                off = 22 if b[:4] == b'\xd7\xcd\xc6\x9a' else 0
                l, p = classify(wmf_blits(b[off:]))
            else:
                l, p = classify(emf_blits(b))
        elif kind == 'wmf':
            l, p = classify(wmf_blits(b))
        else:
            l, p = classify(emf_blits(b))
        lone += l; paired += p
    return lone, paired

if __name__ == '__main__':
    root = pathlib.Path(sys.argv[1])
    rows = []
    for f in sorted(root.rglob('*')):
        if not f.is_file(): continue
        if f.suffix.lower() not in ('.ppt', '.pptx', '.doc', '.docx', '.xls', '.xlsx', '.xlsm'): continue
        try: l, p = scan(f)
        except Exception as e: l, p = -1, -1
        if l or p: rows.append((f.name, f.suffix.lower().lstrip('.'), l, p))
    print('doc\text\tlone_srcand\tpaired')
    for r in rows: print('%s\t%s\t%d\t%d' % r)
    print('# documents with a lone SRCAND:', sum(1 for r in rows if r[2] > 0))
    print('# documents with a paired one :', sum(1 for r in rows if r[3] > 0))
