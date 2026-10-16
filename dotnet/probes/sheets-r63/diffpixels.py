#!/usr/bin/env python3
"""Differing pixels against the reference, before and after, for one page of each document.

The gate cannot see a stroke, so the chart-area border has to be scored on ink.  Rasterises
page 1 of three PDFs — ours before the change, ours after, and the reference — at 100 dpi and
counts pixels that differ from the reference by more than 8 of 255 in any channel.
"""
import os, subprocess, sys, tempfile

def raster(pdf, page, dpi, out):
    subprocess.run(["pdftoppm", "-r", str(dpi), "-f", str(page), "-l", str(page), "-png",
                    "-gray", pdf, out], capture_output=True, timeout=300)
    d = os.path.dirname(out)
    stem = os.path.basename(out)
    for f in sorted(os.listdir(d)):
        if f.startswith(stem) and f.endswith(".png"):
            return os.path.join(d, f)
    return None

def pixels(path):
    import zlib, struct
    data = open(path, "rb").read()
    at, w, h, idat, ctype = 8, 0, 0, b"", 0
    while at < len(data):
        ln = struct.unpack(">I", data[at:at + 4])[0]
        kind = data[at + 4:at + 8]
        body = data[at + 8:at + 8 + ln]
        if kind == b"IHDR":
            w, h, _, ctype = struct.unpack(">IIBB", body[:10])
        if kind == b"IDAT":
            idat += body
        at += 12 + ln
    px = zlib.decompress(idat)
    bpp = {0: 1, 2: 3}[ctype]
    stride = w * bpp
    prev = bytearray(stride)
    rows = []
    i = 0
    for _ in range(h):
        f = px[i]; i += 1
        line = bytearray(px[i:i + stride]); i += stride
        for x in range(stride):
            a = line[x - bpp] if x >= bpp else 0
            b = prev[x]
            c = prev[x - bpp] if x >= bpp else 0
            if f == 1: line[x] = (line[x] + a) & 255
            elif f == 2: line[x] = (line[x] + b) & 255
            elif f == 3: line[x] = (line[x] + (a + b) // 2) & 255
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[x] = (line[x] + pr) & 255
        rows.append(bytes(line)); prev = line
    return w, h, b"".join(rows)

def differing(a, b, tol=8):
    wa, ha, pa = a
    wb, hb, pb = b
    if (wa, ha) != (wb, hb):
        return None
    return sum(1 for x, y in zip(pa, pb) if abs(x - y) > tol)

if __name__ == "__main__":
    before_dir, after_dir, ref_dir, page = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
    names = sys.argv[5:]
    with tempfile.TemporaryDirectory(dir="/c/sandbox/workdir/scratch-r63-sheets") as tmp:
        print("%-50s %10s %10s" % ("document", "before", "after"))
        tb = ta = 0
        for n in names:
            r = raster(os.path.join(ref_dir, n), page, 100, os.path.join(tmp, "r"))
            o1 = raster(os.path.join(before_dir, n), page, 100, os.path.join(tmp, "a"))
            o2 = raster(os.path.join(after_dir, n), page, 100, os.path.join(tmp, "b"))
            if not (r and o1 and o2):
                print("%-50s   no raster" % n[:50]); continue
            R, A, B = pixels(r), pixels(o1), pixels(o2)
            db, da = differing(A, R), differing(B, R)
            tb += db or 0; ta += da or 0
            print("%-50s %10s %10s" % (n[:50], db, da))
            for f in os.listdir(tmp):
                os.remove(os.path.join(tmp, f))
        print("%-50s %10d %10d" % ("TOTAL", tb, ta))
