#!/usr/bin/env python3
"""Every cropped Escher picture in a corpus track, with the kind of blip it points at.

    blip-kind-census.py <corpus-root> [words|sheets|slides]

`crop-wiring-01` established which shapes carry a crop. This adds the one column that
turned out to decide whether LibreOffice applies it on the `.doc` path: **what kind of
picture the `pib` resolves to**.

Resolving it takes two different lookups, because a `.doc` stores its pictures in two
places:

  * an **inline** picture's `OfficeArtInlineSpContainer` is followed immediately by its
    own `OfficeArtFBSE`, so the kind is the byte after that record's header;
  * a **floating** shape's `pib` is a 1-based index into the `OfficeArtBStoreContainer`
    inside the `DggContainer` that `fcDggInfo` names.

Both are walked, never regexed, and the `dgglbl` byte that hid every floating shape from
`crop-wiring-01`'s first scanner is handled by scanning for validated `SpContainer`
headers rather than trusting record lengths.

Control columns are printed for the same reason they were there before: a walker that
reached no shapes, or resolved no kinds, would report a clean answer for a reason that
has nothing to do with the corpus.
"""
import struct
import sys
from pathlib import Path

import olefile

SP_CONTAINER = 0xF004
SP = 0xF00A
OPT_RECORDS = (0xF00B, 0xF121, 0xF122)
BSTORE = 0xF001
BSE = 0xF007

CROP_IDS = {256: "top", 257: "bottom", 258: "left", 259: "right"}
PICTURE_ID = 260

WIN_TYPE = {0: "ERROR", 1: "UNKNOWN", 2: "EMF", 3: "WMF", 4: "PICT", 5: "JPEG",
            6: "PNG", 7: "DIB", 8: "TIFF", 9: "CMYKJPEG"}


def signed(v):
    return v - 2**32 if v >= 2**31 else v


def children(buf, off, end):
    while off + 8 <= end:
        vi, rt, rl = struct.unpack_from("<HHI", buf, off)
        body = off + 8
        stop = end if rl > end - body else body + rl
        yield (vi & 0x0F, vi >> 4, rt, body, stop)
        off = body + rl
        if rl == 0 and rt == 0:
            break


def props_of(buf, off, end):
    props = {}
    for _v, inst, rt, b, s in children(buf, off + 8, end):
        if rt in OPT_RECORDS:
            q = b
            for _ in range(inst):
                if q + 6 > s:
                    break
                pid, val = struct.unpack_from("<HI", buf, q)
                q += 6
                props[pid & 0x3FFF] = val
    return props


def find_containers(buf):
    out, off = [], 0
    while off + 16 <= len(buf):
        vi, rt, rl = struct.unpack_from("<HHI", buf, off)
        if rt == SP_CONTAINER and (vi & 0x0F) == 0x0F and 8 <= rl <= len(buf) - off - 8:
            _v, r2, l2 = struct.unpack_from("<HHI", buf, off + 8)
            if r2 == SP and l2 == 8:
                out.append((off, 8 + rl))
                off += 8 + rl
                continue
        off += 1
    return out


def bse_kind(buf, at):
    """The blip kind of an OfficeArtFBSE record starting at `at`, or None."""
    if at + 16 > len(buf):
        return None
    _vi, rt, _rl = struct.unpack_from("<HHI", buf, at)
    if rt != BSE:
        return None
    return WIN_TYPE.get(buf[at + 8], f"win{buf[at + 8]}")


def store_kinds(blob):
    """The blip kinds of a DggContainer's BStore, 1-based as `pib` indexes them."""
    kinds = []
    for _v, _i, rt, b, s in children(blob, 0, len(blob)):
        if rt == 0xF000:  # DggContainer
            for _v2, _i2, r2, b2, s2 in children(blob, b, s):
                if r2 == BSTORE:
                    p = b2
                    while p + 8 <= s2:
                        _vi, r3, rl3 = struct.unpack_from("<HHI", blob, p)
                        if r3 == BSE:
                            kinds.append(WIN_TYPE.get(blob[p + 8], f"win{blob[p + 8]}"))
                        p += 8 + rl3
            break
    return kinds


def doc_office_art(ole):
    fib = ole.openstream("WordDocument").read()
    if len(fib) < 0x100:
        return b""
    flags = struct.unpack_from("<H", fib, 0x0A)[0]
    table_name = "1Table" if (flags & 0x0200) else "0Table"
    if not ole.exists(table_name):
        return b""
    p = 32
    csw = struct.unpack_from("<H", fib, p)[0]
    p += 2 + csw * 2
    cslw = struct.unpack_from("<H", fib, p)[0]
    p += 2 + cslw * 4
    cb = struct.unpack_from("<H", fib, p)[0]
    p += 2
    if cb <= 50:
        return b""
    fc, lcb = struct.unpack_from("<II", fib, p + 50 * 8)
    if lcb == 0:
        return b""
    return ole.openstream(table_name).read()[fc:fc + lcb]


def report(path):
    try:
        ole = olefile.OleFileIO(str(path))
    except Exception:
        return None
    if not ole.exists("WordDocument"):
        return None

    rows = []
    shapes = pictures = 0

    # inline: the Data stream, each container followed by its own FBSE
    if ole.exists("Data"):
        data = ole.openstream("Data").read()
        for off, length in find_containers(data):
            shapes += 1
            props = props_of(data, off, off + length)
            if props.get(PICTURE_ID, 0):
                pictures += 1
            crops = {CROP_IDS[k]: signed(v) / 65536.0
                     for k, v in props.items() if k in CROP_IDS and signed(v)}
            if crops:
                rows.append(("inline", bse_kind(data, off + length) or "?",
                             props.get(PICTURE_ID, 0), crops))

    # floating: the fcDggInfo blob, pib indexing the DggContainer's BStore
    blob = doc_office_art(ole)
    if blob:
        kinds = store_kinds(blob)
        for off, length in find_containers(blob):
            shapes += 1
            props = props_of(blob, off, off + length)
            pib = props.get(PICTURE_ID, 0)
            if pib:
                pictures += 1
            crops = {CROP_IDS[k]: signed(v) / 65536.0
                     for k, v in props.items() if k in CROP_IDS and signed(v)}
            if crops:
                kind = kinds[pib - 1] if 1 <= pib <= len(kinds) else ("none" if not pib else "?")
                rows.append(("floating", kind, pib, crops))
    return rows, shapes, pictures, len(store_kinds(blob)) if blob else 0


def main():
    root = Path(sys.argv[1])
    track = sys.argv[2] if len(sys.argv) > 2 else "words"
    files = sorted(p for p in (root / track).rglob("*") if p.is_file())

    by_kind = {}
    docs = totals = binaries = all_shapes = all_pictures = 0
    for path in files:
        out = report(path)
        if out is None:
            continue
        binaries += 1
        rows, shapes, pictures, store = out
        all_shapes += shapes
        all_pictures += pictures
        if not rows:
            continue
        docs += 1
        totals += len(rows)
        print(f"{path.name}  ({shapes} shapes, {pictures} with a pib, {store} in the store)")
        for where, kind, pib, crops in rows:
            key = (where, kind)
            by_kind[key] = by_kind.get(key, 0) + 1
            frac = " ".join(f"{k}={v:.4f}" for k, v in crops.items())
            print(f"    {where:8} {kind:8} pib={pib:<4} {frac}")

    print(f"\nCONTROL  {binaries} .doc read, {all_shapes} Escher shapes, "
          f"{all_pictures} carrying a pib")
    print(f"CROPPED  {docs} documents / {totals} shapes")
    for key in sorted(by_kind):
        print(f"    {key[0]:8} {key[1]:8} {by_kind[key]}")


if __name__ == "__main__":
    main()
