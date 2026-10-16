#!/usr/bin/env python3
"""The faces, sizes and font-table reading the three device probes share.

The face list is `probes/lineheight-01/probe-grid.py`'s, unchanged, so a device found here is
measured on exactly the pairs Writer's was — five faces for the main table, eight more that no
round has touched for the falsification set.
"""
import math
import struct

L = "/usr/share/fonts/truetype/liberation/"
X = "/usr/share/fonts/truetype/crosextra/"
D = "/usr/share/fonts/truetype/dejavu/"

FACES = {
    "core": [
        ("Liberation Serif", "Liberation Serif", "", L + "LiberationSerif-Regular.ttf"),
        ("Liberation Sans", "Liberation Sans", "", L + "LiberationSans-Regular.ttf"),
        ("Carlito", "Carlito", "", X + "Carlito-Regular.ttf"),
        ("Caladea", "Caladea", "", X + "Caladea-Regular.ttf"),
        ("DejaVu Sans", "DejaVu Sans", "", D + "DejaVuSans.ttf"),
    ],
    "extra": [
        ("Liberation Mono", "Liberation Mono", "", L + "LiberationMono-Regular.ttf"),
        ("Lib Serif Bold", "Liberation Serif", "bold", L + "LiberationSerif-Bold.ttf"),
        ("Lib Sans Italic", "Liberation Sans", "italic", L + "LiberationSans-Italic.ttf"),
        ("Caladea Bold", "Caladea", "bold", X + "Caladea-Bold.ttf"),
        ("DejaVu Serif", "DejaVu Serif", "", D + "DejaVuSerif.ttf"),
        ("DejaVu Sans Mono", "DejaVu Sans Mono", "", D + "DejaVuSansMono.ttf"),
        ("OpenSymbol", "OpenSymbol", "", "/usr/share/fonts/truetype/libreoffice/opens___.ttf"),
        ("IPAGothic", "IPAGothic", "", "/usr/share/fonts/truetype/fonts-japanese-gothic.ttf"),
    ],
}

SIZES = [h / 2.0 for h in range(10, 49)]          # 5.0 .. 24.0 pt in half points


def metrics(path):
    """The three numbers `ImplCalcLineSpacing` believes, and the em they are in."""
    d = open(path, "rb").read()
    off = 0
    if d[:4] == b"ttcf":
        off, = struct.unpack(">I", d[12:16])
    num, = struct.unpack(">H", d[off + 4:off + 6])
    t = {}
    for i in range(num):
        rec = d[off + 12 + 16 * i: off + 12 + 16 * i + 16]
        o, ln = struct.unpack(">II", rec[8:16])
        t[rec[:4].decode("latin1")] = (o, ln)
    ho, _ = t["head"]
    upem, = struct.unpack(">H", d[ho + 18:ho + 20])
    hh, _ = t["hhea"]
    asc, desc, gap = struct.unpack(">hhh", d[hh + 4:hh + 10])
    a, dsc, g = asc, -desc, gap
    if "OS/2" in t:
        oo, _ = t["OS/2"]
        fs, = struct.unpack(">H", d[oo + 62:oo + 64])
        tA, tD, tG = struct.unpack(">hhh", d[oo + 68:oo + 74])
        if (fs >> 7) & 1 and tA >= 0 and tD <= 0:
            a, dsc, g = tA, -tD, tG
    return upem, a, dsc, g


MET = {f[0]: metrics(f[3]) for s in FACES.values() for f in s}


def rnd(x):
    """C++ std::round / llround: half away from zero."""
    return math.floor(x + 0.5) if x >= 0 else -math.floor(-x + 0.5)


def size_mm100(pt):
    """The em as a 1/100 mm item set holds it."""
    return rnd(pt * 2540.0 / 72.0)
