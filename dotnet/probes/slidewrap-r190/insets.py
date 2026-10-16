#!/usr/bin/env python3
"""Sweep the MASTER body placeholder's right inset and ask 26.2.4.2 where it breaks the line.

    ./insets.py <outdir> [right-inset-in-points ...]

`widths.py`'s sibling, and the one that matters: slide 37's body placeholder takes its insets
from the master, which states `lIns="0" tIns="45720" rIns="91440" bIns="45720"` — a **7.2 pt
right inset** that the first cut of this probe did not know about and that changes what the
width sweep means. Rewriting that one attribute and nothing else brackets how much MORE than its
visible text a line has to be given before the reference will break it.

Both renderers honour the inset, which is what makes the bracket a statement about the break
rule rather than about the box: at 14.4 pt both break the line and at 0 both keep as much of it
as their own rule allows.
"""
import pathlib, sys, zipfile

SRC = ('/home/user/sample-files/slides/done-012/pptx/'
       'Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx')
OLD = ('<a:bodyPr vert="horz" lIns="0" tIns="45720" rIns="91440" bIns="45720" '
       'rtlCol="0">')

out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
insets = [float(a) for a in sys.argv[2:]] or [0.0, 0.5, 1.0, 3.6, 4.5, 5.4, 6.3, 7.2, 14.4]
out.mkdir(parents=True, exist_ok=True)

master = zipfile.ZipFile(SRC).read('ppt/slideMasters/slideMaster1.xml').decode('utf-8')
assert OLD in master

for inset in insets:
    emu = int(round(inset * 12700))
    target = out / f'r-{inset:g}.pptx'
    src = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as zo:
        for item in src.infolist():
            data = src.read(item.filename)
            if item.filename == 'ppt/slideMasters/slideMaster1.xml':
                data = master.replace(
                    OLD, OLD.replace('rIns="91440"', f'rIns="{emu}"'), 1).encode('utf-8')
            zo.writestr(item, data)
    print(f'{target}  rIns {inset:g} pt  {emu} EMU')
