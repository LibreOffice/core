#!/usr/bin/env python3
"""Census for slides-f-01: a stated `a:noFill` on a series line, and a marker's own `c:spPr`.

Walks OPC parts and parses every one with ElementTree. **No regex touches markup** — the rule
this track paid for twice (`slides-c-01`: an alternation with no backreference measured itself,
16 reported against 1 real).

Two departures from `slides-e-01/census.py`, both deliberate:

1. It counts the `a:noFill` occurrences **directly**, rather than inferring them from what
   currently draws. e-01's census inferred "auto stroke" from the absence of a fill under
   `c:ser/c:spPr/a:ln` and therefore could not see the one deck where the fill present was
   `a:noFill` itself. Counting the element is the fix.
2. Reach is separated from declaration in the output. A declaration is a `c:ser` stating
   `a:noFill`; reach is a series whose `Line` is non-null today *and* whose `Line` is consumed by
   something the renderer draws. The two columns are never added together.

Chart parts are reached by transitive relationship walk from `_rels/.rels`, so a chart part
nothing references is not counted.
"""

import sys, os, zipfile, csv
import xml.etree.ElementTree as ET

A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
C = 'http://schemas.openxmlformats.org/drawingml/2006/chart'
CHART_ODF = 'urn:oasis:names:tc:opendocument:xmlns:chart:1.0'

LINEAR = {'lineChart', 'line3DChart', 'scatterChart', 'radarChart', 'stockChart'}
FILLED = {'barChart', 'bar3DChart', 'pieChart', 'pie3DChart', 'doughnutChart',
          'areaChart', 'area3DChart', 'ofPieChart', 'surfaceChart', 'surface3DChart',
          'bubbleChart'}


def parse(z, name):
    try:
        with z.open(name) as f:
            return ET.parse(f).getroot()
    except Exception:
        return None


def rels_of(z, part):
    d, b = os.path.split(part)
    name = f'{d}/_rels/{b}.rels' if d else f'_rels/{b}.rels'
    root = parse(z, name)
    if root is None:
        return []
    return [(r.get('Type', ''), r.get('Target', ''), r.get('TargetMode', '')) for r in root]


def resolve(part, target):
    if target.startswith('/'):
        return target[1:]
    return os.path.normpath(os.path.join(os.path.dirname(part), target)).replace('\\', '/')


def reachable_charts(z):
    """Every chart part reachable from the package root by following relationships."""
    seen, charts, queue = set(), [], ['']
    while queue:
        part = queue.pop()
        if part in seen:
            continue
        seen.add(part)
        for typ, tgt, mode in rels_of(z, part):
            if mode == 'External' or not tgt:
                continue
            child = resolve(part, tgt)
            if typ.endswith('/chart') and child not in charts:
                charts.append(child)
            if child not in seen:
                queue.append(child)
    return charts


def style_of(root):
    for el in root.iter(f'{{{C}}}style'):
        try:
            n = int(el.get('val'))
        except (TypeError, ValueError):
            continue
        if 1 <= n <= 48:
            return n
    return 2


def auto_stroke_visible(style, linear):
    """Whether the automatic table gives this series a non-null stroke colour today.

    `DrawingChartAutoFormat.LinearSeriesLines` is a colour at every style; `FilledSeriesLines`
    is `Invisible` outside 9..16 and 33..40, so a filled series at the default style 2 already
    has a null `Line` and a stated `a:noFill` on it changes nothing.
    """
    return True if linear else (9 <= style <= 16 or 33 <= style <= 40)


def line_child(sp):
    if sp is None:
        return None
    return sp.find(f'{{{A}}}ln')


def paint_kind(el):
    """Which fill element a node states directly, or None."""
    if el is None:
        return None
    for kind in ('solidFill', 'gradFill', 'pattFill', 'blipFill', 'noFill'):
        if el.find(f'{{{A}}}{kind}') is not None:
            return kind
    return None


def census_chart(z, part, row):
    root = parse(z, part)
    if root is None:
        return
    row['charts'] += 1
    style = style_of(root)
    for group in root.iter():
        gname = group.tag.split('}')[-1]
        if gname not in LINEAR and gname not in FILLED:
            continue
        linear = gname in LINEAR
        scatter_style = None
        if gname == 'scatterChart':
            el = group.find(f'{{{C}}}scatterStyle')
            scatter_style = el.get('val') if el is not None else None
        for ser in group.findall(f'{{{C}}}ser'):
            sp = ser.find(f'{{{C}}}spPr')
            ln = line_child(sp)
            marker = ser.find(f'{{{C}}}marker')
            symbol = marker.find(f'{{{C}}}symbol') if marker is not None else None
            symbol = symbol.get('val') if symbol is not None else None
            has_marker = symbol not in ('none',) and (
                symbol is not None
                or (linear and not (gname == 'scatterChart' and scatter_style in ('line', 'smooth'))
                    and gname != 'radarChart'))

            if paint_kind(ln) == 'noFill':
                row['nofill_ser'] += 1
                row['nofill_kinds'].add(gname)
                if auto_stroke_visible(style, linear):
                    row['nofill_reaching'] += 1
                    if has_marker:
                        row['nofill_with_marker'] += 1
                    if gname == 'radarChart':
                        row['nofill_radar'] += 1
                    if not linear:
                        row['nofill_frame_border'] += 1

            msp = marker.find(f'{{{C}}}spPr') if marker is not None else None
            if msp is not None:
                fill = paint_kind(msp)
                mln = paint_kind(line_child(msp))
                if fill or mln:
                    row['marker_sppr'] += 1
                    row['marker_detail'].add(f'{gname}/{symbol}:fill={fill},ln={mln}')

            for dpt in ser.findall(f'{{{C}}}dPt'):
                dm = dpt.find(f'{{{C}}}marker')
                if dm is not None and dm.find(f'{{{C}}}spPr') is not None:
                    row['dpt_marker_sppr'] += 1
                if paint_kind(line_child(dpt.find(f'{{{C}}}spPr'))) == 'noFill':
                    row['dpt_nofill'] += 1


def census_one(path):
    row = {'charts': 0, 'nofill_ser': 0, 'nofill_reaching': 0, 'nofill_with_marker': 0,
           'nofill_radar': 0, 'nofill_frame_border': 0, 'marker_sppr': 0,
           'dpt_marker_sppr': 0, 'dpt_nofill': 0, 'odf_charts': 0,
           'nofill_kinds': set(), 'marker_detail': set()}
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return None
    names = set(z.namelist())
    if '[Content_Types].xml' in names:
        for part in reachable_charts(z):
            census_chart(z, part, row)
    else:
        # ODF: a chart is an embedded object directory with its own content.xml.
        for name in names:
            if not name.endswith('content.xml'):
                continue
            root = parse(z, name)
            if root is not None and root.find(f'.//{{{CHART_ODF}}}chart') is not None:
                row['odf_charts'] += 1
    return row


COLS = ['charts', 'nofill_ser', 'nofill_reaching', 'nofill_with_marker', 'nofill_radar',
        'nofill_frame_border', 'marker_sppr', 'dpt_marker_sppr', 'dpt_nofill', 'odf_charts']


def main(root, out):
    files = []
    for dirpath, _dirs, names in os.walk(root):
        for f in sorted(names):
            p = os.path.join(dirpath, f)
            files.append((os.path.relpath(p, root), p))
    files.sort()

    w = csv.writer(open(out, 'w', newline=''), delimiter='\t')
    w.writerow(['doc'] + COLS + ['nofill_kinds', 'marker_detail'])
    totals = dict.fromkeys(COLS, 0)
    docs = zips = 0
    hits = {c: 0 for c in COLS}
    for rel, p in files:
        r = census_one(p)
        docs += 1
        if r is None:
            w.writerow([rel] + ['n/a'] * len(COLS) + ['not-a-zip', ''])
            continue
        zips += 1
        for c in COLS:
            totals[c] += r[c]
            if r[c]:
                hits[c] += 1
        if any(r[c] for c in COLS):
            w.writerow([rel] + [r[c] for c in COLS]
                       + [' '.join(sorted(r['nofill_kinds'])), ' | '.join(sorted(r['marker_detail']))[:200]])
    print(f'documents {docs}, zip containers {zips}')
    for c in COLS:
        print(f'  {c:22} total {totals[c]:5}   documents {hits[c]:4}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
