#!/usr/bin/env python3
"""Discriminating arms for the OOXML automatic gridline format.

Patches ONE thing at a time in a real corpus deck whose chart states empty
<c:majorGridlines/> and <c:minorGridlines/> on its category axis, so no arm states the
reference's own default and the arms differ in nothing else.

  base   unmodified                       — the control
  dk     theme dk1 000000 -> 2050C0       — "tint of tx1" vs "a constant grey"
  wh     theme dk1 000000 -> FFFFFF       — tint of white is white, whatever the law
  w38    lnStyleLst[0] w 9525 -> 38100    — "the subtle line style's width" vs "0.75 pt"
  rel    lnStyleLst[0] w 9525 -> 4763     — half again, to check the 100% relative factor
"""
import os, re, shutil, subprocess, sys, zipfile

SRC = "/c/sandbox/workdir/sample-files/slides/ceiling-002/pptx/Demick_JetBlue.pptx"
OUT = "/c/sandbox/workdir/scratch-r59-slides/gridprobe"

ARMS = {
    "base": [],
    "dk":   [("ppt/theme/theme1.xml",
              '<a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>',
              '<a:dk1><a:srgbClr val="2050C0"/></a:dk1>')],
    "wh":   [("ppt/theme/theme1.xml",
              '<a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>',
              '<a:dk1><a:srgbClr val="FFFFFF"/></a:dk1>')],
    "w38":  [("ppt/theme/theme1.xml",
              '<a:lnStyleLst><a:ln w="9525"', '<a:lnStyleLst><a:ln w="38100"')],
    "rel":  [("ppt/theme/theme1.xml",
              '<a:lnStyleLst><a:ln w="9525"', '<a:lnStyleLst><a:ln w="4763"')],
}


def build(name, edits):
    dst = os.path.join(OUT, name + ".pptx")
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            for path, old, new in edits:
                if item.filename == path:
                    text = data.decode("utf-8")
                    if old not in text:
                        raise SystemExit(f"{name}: pattern absent in {path}: {old[:60]}")
                    if text.count(old) != 1:
                        raise SystemExit(f"{name}: pattern x{text.count(old)} in {path}")
                    data = text.replace(old, new).encode("utf-8")
            zout.writestr(item, data)
    zin.close()
    return dst


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for name, edits in ARMS.items():
        p = build(name, edits)
        print("built", p, os.path.getsize(p))
