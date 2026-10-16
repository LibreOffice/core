#!/usr/bin/env python3
"""How many corpus documents hold an EMF with a BezierTo record inside a recorded path.

A ceiling, not a reach: a metafile can carry the record and still draw the same picture
either way when each figure happens to be one record long.  Reads zip containers directly
and inflates every plausible stream of an OLE2 container, because a .ppt keeps its blips
zlib-compressed inside Escher records and a raw signature search finds nothing there.
"""
import glob, os, struct, sys, zipfile, zlib

BEGINPATH, ENDPATH = 59, 60
BEZIER_TO, BEZIER_TO16 = 5, 88


def scan(data):
    """(records seen, BezierTo records inside BeginPath/EndPath) for one EMF blob."""
    if len(data) < 88 or data[:4] != b'\x01\x00\x00\x00':
        return 0, 0
    off, seen, inside, recording = 0, 0, 0, False
    while off + 8 <= len(data) and seen < 200000:
        t, sz = struct.unpack_from('<II', data, off)
        if sz < 8 or off + sz > len(data):
            break
        if t == BEGINPATH:
            recording = True
        elif t == ENDPATH:
            recording = False
        elif recording and t in (BEZIER_TO, BEZIER_TO16):
            inside += 1
        off += sz
        seen += 1
    return seen, inside


def blobs(path):
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        raw = open(path, 'rb').read()
        # Inflate every deflate stream we can find; a .ppt's blips are zlib-wrapped.
        for i in range(len(raw) - 2):
            if raw[i] != 0x78 or raw[i + 1] not in (0x01, 0x5E, 0x9C, 0xDA):
                continue
            try:
                out = zlib.decompressobj().decompress(raw[i:], 40_000_000)
            except Exception:
                continue
            if len(out) > 88:
                yield out
        return
    for name in z.namelist():
        if name.lower().endswith(('.emf', '.wmf', '.bin')):
            try:
                yield z.read(name)
            except Exception:
                pass


def main(root):
    hits = []
    for f in sorted(glob.glob(root + '/**/*', recursive=True)):
        if not os.path.isfile(f):
            continue
        total = 0
        for blob in blobs(f):
            total += scan(blob)[1]
        if total:
            hits.append((os.path.basename(f), total))
    print(f'{len(hits)} documents carry a BezierTo inside a recorded path')
    for name, n in sorted(hits, key=lambda r: -r[1]):
        print(f'{n:8d}  {name}')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/workspace/sample-files/slides')
