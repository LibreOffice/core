#!/usr/bin/env python3
"""Read every text show's (page, y, /Tf size) straight out of a PDF content stream.

Deliberately not poppler: a font size is what settles this and poppler never reports one.
"""
import re
import sys
import zlib

MM100 = 2540.0 / 72.0


def pages(path):
    data = open(path, "rb").read()
    out = []
    for m in re.finditer(rb"stream\r?\n", data):
        s = m.end()
        e = data.find(b"endstream", s)
        raw = data[s:e]
        try:
            t = zlib.decompress(raw)
        except Exception:
            continue
        if b"Tf" in t and b"BT" in t:
            out.append(t.decode("latin1"))
    return out


TD = re.compile(r"([-\d.]+)\s+([-\d.]+)\s+Td\s*/(\S+)\s+([\d.]+)\s+Tf")


def main():
    for path in sys.argv[1:]:
        for i, t in enumerate(pages(path), 1):
            rows = [(float(a), float(b), f, float(sz)) for a, b, f, sz in TD.findall(t)]
            rows.sort(key=lambda r: (-r[1]))
            print(f"--- {path} page {i}: {len(rows)} shows")
            prev = None
            for x, y, f, sz in rows:
                d = "" if prev is None else f"  pitch {prev - y:8.3f} pt = {(prev - y) * MM100:8.2f} mm100"
                print(f"  y {y:9.3f}  size {sz:7.3f} pt = {sz * MM100:7.2f} mm100{d}")
                prev = y


main()
