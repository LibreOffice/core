#!/usr/bin/env python3
"""What `c:dLblPos` do the corpus's PIE charts state, and how many points does each reach?

A pie's label placement is the one that goes through PolarLabelPositionHelper's INSIDE/OUTSIDE
arm; every other chart type takes a different placer. So count the value that survives the merge
-- a point's own c:dLbl over its series' c:dLbls over the group's -- inside a c:pieChart,
c:pie3DChart, c:doughnutChart or c:ofPieChart, and nothing else.
"""
import collections, pathlib, re, sys, zipfile
import xml.etree.ElementTree as ET

C = '{http://schemas.openxmlformats.org/drawingml/2006/chart}'
PIE = {'pieChart', 'pie3DChart', 'doughnutChart', 'ofPieChart'}
ROOT = pathlib.Path('/home/user/sample-files')

def pos(el):
    if el is None: return None
    p = el.find(f'{C}dLblPos')
    return p.get('val') if p is not None else None

counts = collections.Counter()
docs = collections.defaultdict(set)

for path in sorted(ROOT.rglob('*')):
    if not path.is_file() or path.suffix.lower() not in ('.docx', '.xlsx', '.xlsm', '.pptx'):
        continue
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        continue
    for name in z.namelist():
        if not re.match(r'(word|xl|ppt)/charts/chart\d*\.xml$', name):
            continue
        try:
            root = ET.fromstring(z.read(name))
        except Exception:
            continue
        for group in root.iter():
            tag = group.tag[len(C):] if group.tag.startswith(C) else None
            if tag not in PIE: continue
            group_pos = pos(group.find(f'{C}dLbls'))
            for ser in group.findall(f'{C}ser'):
                labels = ser.find(f'{C}dLbls')
                ser_pos = pos(labels) or group_pos
                points = ser.find(f'{C}val/{C}numRef/{C}numCache')
                n = points.find(f'{C}ptCount') if points is not None else None
                total = int(n.get('val')) if n is not None else 0
                own = {}
                if labels is not None:
                    for one in labels.findall(f'{C}dLbl'):
                        idx = one.find(f'{C}idx')
                        if idx is None: continue
                        own[int(idx.get('val'))] = pos(one) or ser_pos
                for at in range(total):
                    counts[own.get(at, ser_pos)] += 1
                    docs[own.get(at, ser_pos)].add(str(path.relative_to(ROOT)))

for value, n in counts.most_common():
    print(f'{str(value):10s} {n:6d} points in {len(docs[value]):3d} documents')
