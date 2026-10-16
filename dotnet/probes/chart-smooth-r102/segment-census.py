#!/usr/bin/env python3
"""How many line segments 26.2.4.2 strokes per data interval, on every smoothed witness.

The granularity constant read out of the 27.2 C++ tree (AreaChart.cxx:64,
ScatterChartTypeTemplate.cxx:64) is a hypothesis about the 26.2.4.2 binary, not a fact about
it.  This counts the `l` operators of every long stroked polyline in the reference bank's own
PDFs and divides by the intervals the chart part states.

Run: python3 segment-census.py > segments.tsv
"""
import pymupdf, sys, zipfile, re, pathlib

BANK = pathlib.Path('/home/user/gate-orig-r83/ref')
CORPUS = pathlib.Path('/home/user/sample-files')

# document -> (reference PDF stem, chart part, points per smoothed series)
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
    ('sheets/chartset-008/xlsx/055_Project_timeline_with_milestones_Use_this_template_546cecc0.xlsx',
     '055_Project_timeline_with_milestones_Use_this_template_546cecc0__xlsx'),
    ('slides/done-011/pptx/171128IPAP.pptx', '171128IPAP__pptx'),
]


def polylines(stem):
    """Every stroked all-straight path of eight segments or more, once per page."""
    d = pymupdf.open(BANK / f'{stem}.pdf')
    seen = set()
    out = []
    for pno in range(d.page_count):
        for dr in d[pno].get_drawings():
            if dr['type'] != 's':
                continue
            items = dr['items']
            if len(items) < 8 or any(i[0] != 'l' for i in items):
                continue
            key = (len(items), tuple(round(v, 2) for v in dr['rect'][2:]))
            if key in seen:
                continue
            seen.add(key)
            out.append(len(items))
    return sorted(out)


print('document\tsegments per stroked polyline (distinct)')
for rel, stem in WITNESSES:
    counts = polylines(stem)
    tally = {}
    for c in counts:
        tally[c] = tally.get(c, 0) + 1
    print(f"{rel}\t{', '.join(f'{k}x{v}' for k, v in sorted(tally.items()))}")
