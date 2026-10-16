#!/usr/bin/env python3
"""Why nine of the eleven `c:smooth val="1"` witnesses cannot move.

Their series state values in arithmetic progression against evenly spaced categories, and a
natural cubic spline through collinear points is the chord.  This measures it at the reference
rather than asserting it: for every flattened polyline 26.2.4.2 strokes in those documents, how
far the curve departs from the straight line joining its own two ends.

Run: python3 collinear.py > collinear.tsv
"""
import math, pathlib, pymupdf, re, zipfile

BANK = pathlib.Path('/home/user/gate-orig-r83/ref')
CORPUS = pathlib.Path('/home/user/sample-files')

WITNESSES = [
    ('sheets/chartset-001/xlsx/002_advanced_excel_line.xlsx', '002_advanced_excel_line__xlsx'),
    ('sheets/chartset-001/xlsx/006_advanced_excel_scatter.xlsx', '006_advanced_excel_scatter__xlsx'),
    ('sheets/chartset-001/xlsx/022_advanced_excel_scatter.xlsx', '022_advanced_excel_scatter__xlsx'),
    ('sheets/chartset-002/xlsx/026_advanced_excel_line.xlsx', '026_advanced_excel_line__xlsx'),
    ('sheets/chartset-002/xlsx/030_advanced_excel_scatter.xlsx', '030_advanced_excel_scatter__xlsx'),
    ('sheets/chartset-002/xlsx/034_advanced_excel_line.xlsx', '034_advanced_excel_line__xlsx'),
    ('sheets/chartset-003/xlsx/014_advanced_excel_scatter.xlsx', '014_advanced_excel_scatter__xlsx'),
    ('sheets/chartset-003/xlsx/018_advanced_excel_line.xlsx', '018_advanced_excel_line__xlsx'),
    ('sheets/chartset-004/xlsx/010_advanced_excel_line.xlsx', '010_advanced_excel_line__xlsx'),
    ('sheets/chartset-004/xlsx/microsoft_learn_multi_chart_examples.xlsx',
     'microsoft_learn_multi_chart_examples__xlsx'),
]

VALUES = re.compile(r'<(?:\w+:)?(?:val|yVal)>(.*?)</(?:\w+:)?(?:val|yVal)>', re.S)
POINT = re.compile(r'<(?:\w+:)?v>([-\d.eE+]+)</(?:\w+:)?v>')


def arithmetic(document):
    """Whether every cached value sequence in the document rises by a constant."""
    z = zipfile.ZipFile(CORPUS / document)
    answers = []
    for name in z.namelist():
        if 'chart' not in name.lower() or not name.endswith('.xml'):
            continue
        text = z.read(name).decode('utf8', 'replace')
        for block in VALUES.finditer(text):
            values = [float(v) for v in POINT.findall(block.group(1))]
            if len(values) < 3:
                continue
            steps = {round(values[i + 1] - values[i], 9) for i in range(len(values) - 1)}
            answers.append(len(steps) == 1)
    return answers


def straightness(pdf):
    """Every long stroked polyline's greatest departure from its own chord, in points."""
    d = pymupdf.open(BANK / f'{pdf}.pdf')
    worst = []
    for pno in range(d.page_count):
        for dr in d[pno].get_drawings():
            items = dr['items']
            if dr['type'] != 's' or len(items) < 40 or any(i[0] != 'l' for i in items):
                continue
            pts = [(items[0][1].x, items[0][1].y)] + [(i[2].x, i[2].y) for i in items]
            (x0, y0), (x1, y1) = pts[0], pts[-1]
            dx, dy = x1 - x0, y1 - y0
            length = math.hypot(dx, dy)
            if length == 0:
                continue
            worst.append(max(abs(((p[0] - x0) * dy) - ((p[1] - y0) * dx)) / length for p in pts))
    return worst


print('document\tvalue sequences arithmetic\tworst departure from the chord (pt)')
for rel, pdf in WITNESSES:
    flags = arithmetic(rel)
    worst = straightness(pdf)
    print(f'{rel}\t{sum(flags)} of {len(flags)}\t'
          f'{max(worst):.4f}' if worst else f'{rel}\t{sum(flags)} of {len(flags)}\t(none drawn)')
