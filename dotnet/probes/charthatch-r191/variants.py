#!/usr/bin/env python3
"""One-attribute variants of 028's chart-area `a:pattFill/@prst`.

The question is what unit LibreOffice reads a hatch's distance in: the four presets state 50, 100,
50 and 25 hundredths of a millimetre at two different angles, so a single factor that fits all
four is the unit and anything angle-dependent is not.

Rendered with 26.2.4.2 and read with `spacing.py`, which counts the strokes of the hatch's own
colour and divides the box's projected extent by them — the robust way, because a `pdf-ops.py`
stroke record is a bounding BOX and for one of the two diagonal senses its corners are not the
segment's endpoints.
"""
import pathlib
import zipfile

SRC = pathlib.Path(
    '/home/user/sample-files/words/chartset-010/docx/'
    '028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx')
OUT = pathlib.Path(__file__).parent / 'var'
PART = 'word/charts/chart1.xml'

# preset -> the Distance hatchmap.hxx gives it, in hundredths of a millimetre
PRESETS = {'wdDnDiag': 100, 'ltHorz': 50, 'dkHorz': 25}


def write(preset: str) -> pathlib.Path:
    OUT.mkdir(exist_ok=True)
    target = OUT / f'v-{preset}.docx'
    with zipfile.ZipFile(SRC) as src, zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as dst:
        for item in src.infolist():
            data = src.read(item.filename)
            if item.filename == PART:
                text = data.decode('utf-8')
                assert 'prst="dkDnDiag"' in text
                data = text.replace('prst="dkDnDiag"', f'prst="{preset}"').encode('utf-8')
            dst.writestr(item, data)
    return target


for name, stated in PRESETS.items():
    print(stated, write(name))
