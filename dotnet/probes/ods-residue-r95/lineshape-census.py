#!/usr/bin/env python3
"""Census of the ODF drawing elements whose rectangle is a *pair of points*.

`draw:line`, `draw:connector` and `draw:measure` state no `svg:width`/`svg:height` and
no `svg:x`/`svg:y`: their geometry is `svg:x1`,`svg:y1`,`svg:x2`,`svg:y2`
(`SdXMLLineShapeContext::startFastElement`, `xmloff/source/draw/ximpshap.cxx`:1045-1097,
which sets the shape's position to the minimum of the two points and puts the pair in the
polygon; `SdXMLConnectorShapeContext`, `:1963-2035`, which sets `StartPosition` and
`EndPosition`). A reader that looks only for `svg:width` gives every one of them a
zero-sized box, and a zero-sized box widens no print area.

Reports per document how many of each kind sit in a cell, how many are diagonal (both
coordinates differ, so a box outline would be visibly wrong), and how many state a stroke.

Usage: lineshape-census.py <root> [...]
"""
import sys
import zipfile
from pathlib import Path
from xml.etree import ElementTree as ET

DRAW = '{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}'
SVG = '{urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0}'
STYLE = '{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
TABLE = '{urn:oasis:names:tc:opendocument:xmlns:table:1.0}'

KINDS = ('line', 'connector', 'measure')


def content(path):
    if path.suffix.lower() == '.fods':
        return ET.fromstring(path.read_bytes()), None
    with zipfile.ZipFile(path) as z:
        styles = None
        try:
            styles = ET.fromstring(z.read('styles.xml'))
        except KeyError:
            pass
        return ET.fromstring(z.read('content.xml')), styles


def stroked(roots, name):
    """Whether the named graphic style states a stroke other than `none`."""
    if not name:
        return False
    for root in roots:
        if root is None:
            continue
        for style in root.iter(STYLE + 'style'):
            if style.get(STYLE + 'name') != name:
                continue
            for props in style:
                if props.tag == STYLE + 'graphic-properties':
                    return (props.get(DRAW + 'stroke') or 'none') != 'none'
    return False


def scan(path):
    root, styles = content(path)
    counts = dict.fromkeys(KINDS, 0)
    diagonal = 0
    inked = 0
    for table in root.iter(TABLE + 'table'):
        for kind in KINDS:
            for shape in table.iter(DRAW + kind):
                counts[kind] += 1
                x1 = shape.get(SVG + 'x1')
                y1 = shape.get(SVG + 'y1')
                x2 = shape.get(SVG + 'x2')
                y2 = shape.get(SVG + 'y2')
                if None not in (x1, y1, x2, y2) and x1 != x2 and y1 != y2:
                    diagonal += 1
                if stroked((root, styles), shape.get(DRAW + 'style-name')):
                    inked += 1
    return counts, diagonal, inked


def main():
    roots = [Path(a) for a in sys.argv[1:]]
    paths = sorted({p.resolve() for root in roots for p in root.rglob('*')
                    if p.is_file() and p.suffix.lower() in ('.ods', '.fods', '.ots')})
    print('document\tline\tconnector\tmeasure\tdiagonal\tstroked')
    documents = 0
    totals = dict.fromkeys(KINDS, 0)
    all_diagonal = all_inked = 0
    for p in paths:
        try:
            counts, diagonal, inked = scan(p)
        except Exception as exc:                    # noqa: BLE001
            print(f'{p.name}\tERROR\t{exc}\t\t\t')
            continue
        if not any(counts.values()):
            continue
        documents += 1
        for kind in KINDS:
            totals[kind] += counts[kind]
        all_diagonal += diagonal
        all_inked += inked
        print(f'{p.name}\t{counts["line"]}\t{counts["connector"]}\t{counts["measure"]}'
              f'\t{diagonal}\t{inked}')
    print(f'# {documents} of {len(paths)} documents: '
          f'line {totals["line"]}, connector {totals["connector"]}, '
          f'measure {totals["measure"]}; {all_diagonal} diagonal, {all_inked} stroked',
          file=sys.stderr)


if __name__ == '__main__':
    main()
