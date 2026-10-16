#!/usr/bin/env python3
"""Sweep a slide body placeholder's width and ask 26.2.4.2 where it breaks the line.

    ./widths.py <outdir> [right-edge-in-points ...]

Rewrites nothing but `<a:ext cx>` on slide 37 of the Intersil deck -- one attribute, every other
part of the package byte-identical -- so the reference's answer is attributable. The default
sweep brackets the boundary this round measured.

Read the result with `lines.py`, which prints the drawn line and its right edge; the question is
whether the break point sits at the candidate line's width WITH its trailing blank (867.7 pt of
text) or WITHOUT it (858.8).
"""
import pathlib, sys, zipfile

SRC = ('/home/user/sample-files/slides/done-012/pptx/'
       'Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx')
OLD = '<a:ext cx="11012681" cy="4976501"/>'
LEFT = 48.0                      # the placeholder's own x, 609599 EMU

out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
edges = [float(a) for a in sys.argv[2:]] or [906.0, 907.0, 915.0, 915.14, 915.6, 916.14, 924.0]
out.mkdir(parents=True, exist_ok=True)

slide = zipfile.ZipFile(SRC).read('ppt/slides/slide37.xml').decode('utf-8')
assert OLD in slide

for edge in edges:
    cx = int(round((edge - LEFT) * 12700))
    target = out / f'w-{edge:g}.pptx'
    src = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as zo:
        for item in src.infolist():
            data = src.read(item.filename)
            if item.filename == 'ppt/slides/slide37.xml':
                data = slide.replace(
                    OLD, f'<a:ext cx="{cx}" cy="4976501"/>', 1).encode('utf-8')
            zo.writestr(item, data)
    print(f'{target}  right edge {edge:g} pt  cx {cx}')
