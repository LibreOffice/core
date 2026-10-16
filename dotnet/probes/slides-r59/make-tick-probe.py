#!/usr/bin/env python3
"""Does an axis reserve its tick length when it draws no outward tick?

One property, one axis at a time, in a real corpus chart that already states
<c:majorTickMark val="none"/> on both axes -- so no arm states the reference's own default and
the arms differ in nothing else.  The category axis' element comes first in the part and the
value axis' second, which is what the ordinal patch keys on and what `check` asserts.
"""
import os, re, sys, zipfile

SRC = "/c/sandbox/workdir/sample-files/slides/chartset-001/pptx/bar_chart.pptx"
PART = "ppt/charts/chart1.xml"
OUT = "/c/sandbox/workdir/scratch-r59-slides/tickprobe"
TAG = '<c:majorTickMark val="none"/>'

# (name, value for the CATEGORY axis, value for the VALUE axis)
ARMS = [
    ("base", "none", "none"),
    ("catout", "out", "none"),
    ("valout", "none", "out"),
    ("bothout", "out", "out"),
    ("bothin", "in", "in"),
    ("bothcross", "cross", "cross"),
]


def build(name, cat, val):
    dst = os.path.join(OUT, name + ".pptx")
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == PART:
                text = data.decode("utf-8")
                if text.count(TAG) != 2:
                    raise SystemExit(f"expected 2 {TAG}, found {text.count(TAG)}")
                # the category axis is the first <c:catAx> and the value axis the first
                # <c:valAx>; assert the ordinals before relying on them
                if text.index("<c:catAx>") > text.index("<c:valAx>"):
                    raise SystemExit("valAx precedes catAx — the ordinal patch is unsafe")
                out, n = [], 0
                for piece in text.split(TAG):
                    out.append(piece)
                new = [f'<c:majorTickMark val="{cat}"/>', f'<c:majorTickMark val="{val}"/>']
                text = out[0] + new[0] + out[1] + new[1] + out[2]
                data = text.encode("utf-8")
            zout.writestr(item, data)
    zin.close()
    return dst


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for name, cat, val in ARMS:
        print("built", build(name, cat, val))
