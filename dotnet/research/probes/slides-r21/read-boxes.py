#!/usr/bin/env python3
"""Group a page's text shows into columns by x, then report each column's baseline pitches."""
import re
import sys
import zlib

MM100 = 2540.0 / 72.0
TD = re.compile(r"([-\d.]+)\s+([-\d.]+)\s+Td\s*/(\S+)\s+([\d.]+)\s+Tf")


def streams(path):
    data = open(path, "rb").read()
    out = []
    for m in re.finditer(rb"stream\r?\n", data):
        s = m.end()
        e = data.find(b"endstream", s)
        try:
            t = zlib.decompress(data[s:e])
        except Exception:
            continue
        if b"Tf" in t and b"BT" in t:
            out.append(t.decode("latin1"))
    return out


def main():
    for path in sys.argv[1:]:
        for i, t in enumerate(streams(path), 1):
            rows = [(float(a), float(b), float(sz)) for a, b, _f, sz in TD.findall(t)]
            cols = {}
            for x, y, sz in rows:
                key = min(cols, key=lambda k: abs(k - x), default=None)
                if key is None or abs(key - x) > 60:
                    key = x
                cols.setdefault(key, []).append((y, sz))
            print(f"=== {path} page {i}")
            for key in sorted(cols):
                seq = sorted(cols[key], key=lambda r: -r[0])
                sizes = sorted({round(s * MM100) for _y, s in seq})
                print(f"  column x~{key:7.1f}  sizes(mm100) {sizes}  n={len(seq)}")
                prev = None
                pitches = []
                for y, _sz in seq:
                    if prev is not None:
                        pitches.append(round((prev - y) * MM100))
                    prev = y
                print(f"    pitches(mm100) {pitches}")


main()
