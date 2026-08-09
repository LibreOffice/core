#!/usr/bin/env python3
"""Census of per-element chart text sizes across a corpus of OOXML decks.

`ChartPlot` carries one `LabelSize` for every axis, every data label and (when the legend
states none of its own) the legend.  This counts the chart parts where that collapse loses
something: where two axes state different sizes, or a series' `c:dLbls` states a size the
axes do not.

Reach is reported as *chart parts* and *documents*, and as a ceiling rather than as a
measurement — a part that states a second size still only moves ink if the elements
stating it are drawn.
"""
import sys, zipfile, collections
import xml.etree.ElementTree as ET
from pathlib import Path

C = '{http://schemas.openxmlformats.org/drawingml/2006/chart}'
A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'


def size_of(el):
    """The `sz` of the first a:defRPr under el, in points, or None."""
    if el is None:
        return None
    for d in el.iter(A + 'defRPr'):
        sz = d.get('sz')
        if sz:
            return int(sz) / 100.0
    return None


def child(el, name):
    return None if el is None else el.find(C + name)


def census(path):
    out = []
    with zipfile.ZipFile(path) as z:
        for n in z.namelist():
            if not (n.startswith('ppt/charts/chart') and n.endswith('.xml')):
                continue
            try:
                r = ET.fromstring(z.read(n))
            except Exception:
                continue
            chart = child(r, 'chart')
            plot = child(chart, 'plotArea')
            if plot is None:
                continue

            axes = {}
            for ax in plot:
                if not ax.tag.startswith(C) or not ax.tag.endswith('Ax'):
                    continue
                axes[ax.tag[len(C):]] = size_of(child(ax, 'txPr'))

            # What AxisLabelSizeOf answers: the first axis in document order stating one.
            collapsed = next((v for v in axes.values() if v is not None), None)

            dlbls = []
            for group in plot:
                if not group.tag.startswith(C):
                    continue
                for ser in group.findall(C + 'ser'):
                    s = size_of(child(child(ser, 'dLbls'), 'txPr'))
                    if s is not None:
                        dlbls.append(s)
                g = size_of(child(child(group, 'dLbls'), 'txPr'))
                if g is not None:
                    dlbls.append(g)

            legend = size_of(child(child(chart, 'legend'), 'txPr'))

            stated = {v for v in axes.values() if v is not None}
            axis_split = len(stated) > 1
            label_split = any(d != collapsed for d in dlbls) and (dlbls or collapsed)
            out.append((n, collapsed, dict(axes), sorted(set(dlbls)), legend,
                        axis_split, label_split))
    return out


def main(root):
    docs = sorted(p for p in Path(root).rglob('*')
                  if p.suffix.lower() in ('.pptx', '.pptm', '.potx', '.ppsx'))
    parts = 0
    axis_split_parts = axis_split_docs = 0
    label_split_parts = 0
    hit_docs = set()
    label_docs = set()
    for d in docs:
        try:
            rows = census(d)
        except Exception:
            continue
        for (n, collapsed, axes, dlbls, legend, axis_split, label_split) in rows:
            parts += 1
            if axis_split:
                axis_split_parts += 1
                hit_docs.add(d.name)
            if label_split:
                label_split_parts += 1
                label_docs.add(d.name)
            if axis_split or label_split:
                print(f"{d.name}\t{n}\tcollapsed={collapsed}\taxes={axes}\t"
                      f"dLbls={dlbls}\tlegend={legend}")
    print()
    print(f"documents scanned              {len(docs)}")
    print(f"chart parts                    {parts}")
    print(f"axes stating different sizes   {axis_split_parts} parts, "
          f"{len(hit_docs)} documents")
    print(f"dLbls differing from the axes  {label_split_parts} parts, "
          f"{len(label_docs)} documents")
    print(f"either                         {len(hit_docs | label_docs)} documents")


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/workspace/sample-files/slides')
