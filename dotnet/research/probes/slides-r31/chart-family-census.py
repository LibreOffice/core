#!/usr/bin/env python3
"""Where an OOXML chart part names more than one Latin face, and which one wins.

`DrawingChartPlot.FamilyOf` takes the first literal `a:latin/@typeface` *anywhere* in the
part.  Document order is not a precedence rule: a chart whose title states `Arial` and
whose chart-space `c:txPr` states `Calibri` has its axes, legend and data labels measured
and drawn in Arial.

This counts the parts where the two answers differ — "first anywhere" against "the chart
space's own `c:txPr`, then the theme's minor Latin face".
"""
import sys, zipfile, re
import xml.etree.ElementTree as ET
from pathlib import Path

C = '{http://schemas.openxmlformats.org/drawingml/2006/chart}'
A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'


def literal(el):
    """The first literal a:latin typeface under el; `+mn-lt` and friends are references."""
    if el is None:
        return None
    for lat in el.iter(A + 'latin'):
        face = (lat.get('typeface') or '').strip()
        if face and face[0] != '+':
            return face
    return None


def theme_minor(z, part):
    """The minor Latin face of the theme override this chart part names, if any."""
    rels = f"{'/'.join(part.split('/')[:-1])}/_rels/{part.split('/')[-1]}.rels"
    try:
        r = ET.fromstring(z.read(rels))
    except Exception:
        return None
    for rel in r:
        t = rel.get('Target') or ''
        if 'theme' not in t:
            continue
        name = 'ppt/theme/' + t.split('/')[-1]
        try:
            th = ET.fromstring(z.read(name))
        except Exception:
            continue
        for minor in th.iter(A + 'minorFont'):
            return literal(minor)
    return None


def main(root):
    parts = differ = 0
    docs = set()
    for path in sorted(Path(root).rglob('*')):
        if path.suffix.lower() not in ('.pptx', '.pptm', '.potx', '.ppsx'):
            continue
        try:
            z = zipfile.ZipFile(path)
        except Exception:
            continue
        with z:
            for n in z.namelist():
                if not (n.startswith('ppt/charts/chart') and n.endswith('.xml')):
                    continue
                try:
                    r = ET.fromstring(z.read(n))
                except Exception:
                    continue
                parts += 1
                first = literal(r)
                space = literal(r.find(C + 'txPr')) or theme_minor(z, n)
                if first != space:
                    differ += 1
                    docs.add(path.name)
                    print(f"{path.name}\t{n}\tfirst-anywhere={first}\tchart-space={space}")
    print()
    print(f"chart parts                    {parts}")
    print(f"the two rules disagree         {differ} parts, {len(docs)} documents")


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/workspace/sample-files/slides')
