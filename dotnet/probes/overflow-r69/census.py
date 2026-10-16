#!/usr/bin/env python3
"""Four whole-corpus censuses this round leant on.

    census.py <corpus-root> [centring|separators|shapebox|overflow]

`centring`   — sheets stating `printOptions/@horizontalCentered` or `@verticalCentered`,
               which is the surface the print-centring fix moves.
`separators` — worksheet shape text bodies whose `a:t` carries a bare line separator,
               which is the surface the paragraph-break fix moves.
`shapebox`   — what a worksheet `xdr:sp` declares for its own fill and outline. Nothing
               reads any of it yet; this is the scope of the feature the round did NOT do.
`overflow`   — what a worksheet `a:bodyPr` declares for its two overflow attributes. The
               point of it is the `horizontal only` line: `horzOverflow` is inert in
               LibreOffice, and the corpus has no document where reading it could act.

xlsx family only, and that is a real limit rather than a shortcut: the corpus holds no ODF
spreadsheet at all, and the BIFF path states these things in Escher records rather than in
markup. A figure here is a figure about `xlsx`/`xlsm`/`xltx`.
"""
import collections
import os
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
XDR = '{http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing}'
EXTS = ('.xlsx', '.xlsm', '.xltx')


def workbooks(root):
    for dirpath, _, files in os.walk(root):
        for name in sorted(files):
            if name.lower().endswith(EXTS):
                path = os.path.join(dirpath, name)
                try:
                    yield os.path.relpath(path, root), zipfile.ZipFile(path)
                except Exception:
                    continue


def centring(root):
    docs = collections.defaultdict(set)
    sheets = collections.Counter()
    total = 0
    for rel, z in workbooks(root):
        total += 1
        for name in z.namelist():
            if not re.match(r'xl/worksheets/sheet[^/]*\.xml$', name):
                continue
            found = re.search(r'<printOptions[^>]*>', z.read(name).decode('utf8', 'replace'))
            if not found:
                continue
            for axis in ('horizontal', 'vertical'):
                if re.search(axis + r'Centered="(1|true)"', found.group(0)):
                    sheets[axis] += 1
                    docs[axis].add(rel)
    print(f'{total} workbooks scanned')
    for axis in ('horizontal', 'vertical'):
        print(f'  {axis}Centered: {sheets[axis]} sheets in {len(docs[axis])} documents')
    print(f'  either: {len(docs["horizontal"] | docs["vertical"])} documents')


def separators(root):
    docs, bodies, seps, all_bodies, all_docs = set(), 0, 0, 0, set()
    for rel, z in workbooks(root):
        for name in z.namelist():
            if not re.match(r'xl/drawings/drawing[^/]*\.xml$', name):
                continue
            try:
                tree = ET.fromstring(z.read(name))
            except Exception:
                continue
            for body in tree.iter(XDR + 'txBody'):
                all_bodies += 1
                all_docs.add(rel)
                hit = False
                for run in body.iter(A + 't'):
                    if run.text and ('\n' in run.text or '\r' in run.text):
                        seps += run.text.count('\n') + run.text.count('\r')
                        hit = True
                if hit:
                    bodies += 1
                    docs.add(rel)
    print(f'worksheet shape text bodies: {all_bodies} in {len(all_docs)} documents')
    print(f'  holding a line separator in an a:t: {bodies} bodies, '
          f'{len(docs)} documents, {seps} separators')
    for rel in sorted(docs):
        print('   ', rel)


def shapebox(root):
    counts = collections.Counter()
    docs = collections.defaultdict(set)
    presets = collections.Counter()
    colours = collections.Counter()
    for rel, z in workbooks(root):
        for name in z.namelist():
            if not re.match(r'xl/drawings/drawing[^/]*\.xml$', name):
                continue
            try:
                tree = ET.fromstring(z.read(name))
            except Exception:
                continue
            for shape in tree.iter(XDR + 'sp'):
                counts['shapes'] += 1
                docs['shapes'].add(rel)
                if shape.find(XDR + 'style') is not None:
                    counts['xdr:style'] += 1
                    docs['xdr:style'].add(rel)
                properties = shape.find(XDR + 'spPr')
                if properties is None:
                    continue
                geometry = properties.find(A + 'prstGeom')
                if geometry is not None:
                    presets[geometry.get('prst')] += 1
                for tag, label in (('solidFill', 'fill'), ('noFill', 'noFill')):
                    if properties.find(A + tag) is not None:
                        counts[label] += 1
                        docs[label].add(rel)
                filled = properties.find(A + 'solidFill')
                if filled is not None:
                    for child in filled:
                        colours['fill:' + child.tag.replace(A, '')] += 1
                line = properties.find(A + 'ln')
                if line is None:
                    continue
                counts['a:ln'] += 1
                docs['a:ln'].add(rel)
                stroke = line.find(A + 'solidFill')
                if stroke is not None:
                    counts['line'] += 1
                    docs['line'].add(rel)
                    for child in stroke:
                        colours['line:' + child.tag.replace(A, '')] += 1
                if line.find(A + 'noFill') is not None:
                    counts['line-noFill'] += 1
    for key, value in counts.most_common():
        print(f'{key:14s} {value:5d} shapes in {len(docs.get(key, ())):3d} documents')
    print('\npresets:', presets.most_common())
    print('colour references:', colours.most_common())


def overflow(root):
    counts = collections.Counter()
    docs = collections.defaultdict(set)
    for rel, z in workbooks(root):
        for name in z.namelist():
            if not re.match(r'xl/drawings/drawing[^/]*\.xml$', name):
                continue
            try:
                tree = ET.fromstring(z.read(name))
            except Exception:
                continue
            for body in tree.iter(A + 'bodyPr'):
                horizontal, vertical = body.get('horzOverflow'), body.get('vertOverflow')
                for axis, value in (('horz', horizontal), ('vert', vertical)):
                    if value:
                        counts[f'{axis}Overflow="{value}"'] += 1
                        docs[axis].add(rel)
                if horizontal and not vertical:
                    counts['horizontal only'] += 1
                    docs['horizontal only'].add(rel)
    for key, value in counts.most_common():
        axis = key.split('Overflow')[0] if 'Overflow' in key else key
        print(f'{key:22s} {value:4d} bodies in {len(docs.get(axis, ())):3d} documents')
    if not counts['horizontal only']:
        print('\nNo body states horzOverflow without vertOverflow, so reading the horizontal '
              'attribute\ncould not change one corpus document even if LibreOffice read it. '
              'It does not.')


if __name__ == '__main__':
    corpus = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files'
    which = sys.argv[2] if len(sys.argv) > 2 else 'all'
    for label, run in (('centring', centring), ('separators', separators),
                       ('shapebox', shapebox), ('overflow', overflow)):
        if which in (label, 'all'):
            print('==', label)
            run(corpus)
            print()
