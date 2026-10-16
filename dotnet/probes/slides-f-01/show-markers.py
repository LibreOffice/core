#!/usr/bin/env python3
"""Print the c:ser markup that this round is about, for one deck: the series' own spPr line and
its c:marker/c:spPr. Reads the OPC parts with ElementTree; no regex."""
import sys, zipfile
import xml.etree.ElementTree as ET
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from census import reachable_charts, C, A, style_of, LINEAR, FILLED


def show(path):
    z = zipfile.ZipFile(path)
    for part in reachable_charts(z):
        with z.open(part) as f:
            root = ET.parse(f).getroot()
        style = style_of(root)
        for group in root.iter():
            g = group.tag.split('}')[-1]
            if g not in LINEAR and g not in FILLED:
                continue
            for i, ser in enumerate(group.findall(f'{{{C}}}ser')):
                sp = ser.find(f'{{{C}}}spPr')
                marker = ser.find(f'{{{C}}}marker')
                msp = marker.find(f'{{{C}}}spPr') if marker is not None else None
                if sp is None and msp is None:
                    continue
                idx = ser.find(f'{{{C}}}idx')
                print(f'--- {part} style={style} {g} ser#{i} idx={idx.get("val") if idx is not None else "?"}')
                if sp is not None:
                    print('    spPr   :', ET.tostring(sp, encoding='unicode').replace('\n', ' ')[:400])
                if marker is not None:
                    print('    marker :', ET.tostring(marker, encoding='unicode').replace('\n', ' ')[:500])


for p in sys.argv[1:]:
    print(f'===== {p}')
    show(p)
