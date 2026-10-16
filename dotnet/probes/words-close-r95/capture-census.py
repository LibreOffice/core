#!/usr/bin/env python3
"""Every absolutely positioned DOCX object by what it holds and whether it wraps through.

`SwAnchoredObjectPosition`'s constructor (`anchoredobjectposition.cxx`:125-144) decides whether an
object is captured at all:

    mbDoNotCaptureAnchoredObj = bConsidered && !mbFollowTextFlow && DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE
    fly  (picture, OLE/chart, Writer text frame)   bConsidered = wrapThrough && !bTextBox
    draw (shape, group)                            bConsidered = wrapThrough || !bTextBox

With DOCX's DO_NOT_CAPTURE set and no table anchor, an object is captured exactly when
`bConsidered` is false. So the two readings — *captured unless wrap-through* and the C++'s own —
differ only on a **draw object with no text box that is not wrap-through**. This counts those.

Usage: capture-census.py <MANIFEST.tsv>
"""
import collections
import csv
import pathlib
import sys
import zipfile
from xml.etree import ElementTree as ET

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
WP = '{http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing}'
PIC = '{http://schemas.openxmlformats.org/drawingml/2006/picture}'
WPS = '{http://schemas.microsoft.com/office/word/2010/wordprocessingShape}'
WPG = '{http://schemas.microsoft.com/office/word/2010/wordprocessingGroup}'
A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
V = '{urn:schemas-microsoft-com:vml}'


def drawing_kind(anchor):
    tags = {e.tag for e in anchor.iter()}
    if PIC + 'pic' in tags:
        return 'fly-picture'
    if A + 'graphicFrame' in tags or any(t.endswith('}chart') for t in tags):
        return 'fly-ole'
    if WPS + 'txbx' in tags:
        return 'draw-shape+text'
    if WPG + 'wgp' in tags:
        return 'draw-group'
    if WPS + 'wsp' in tags:
        return 'draw-shape'
    return 'other'


def drawing_wrap(anchor):
    for c in anchor:
        n = c.tag.split('}')[1]
        if n.startswith('wrap'):
            return n
    return 'wrapNone'


def vml_kind(shape):
    tags = {e.tag for e in shape.iter()}
    if V + 'textbox' in tags:
        return 'draw-shape+text'
    if V + 'imagedata' in tags:
        return 'fly-picture'
    return 'draw-shape'


def vml_wrap(shape):
    for c in shape:
        if c.tag == W + 'wrap':
            return c.get('type') or 'through'
    style = shape.get('style') or ''
    return 'through' if 'z-index' in style else 'through'


def main():
    manifest = pathlib.Path(sys.argv[1])
    root = manifest.parent
    counts = collections.Counter()
    docs = collections.defaultdict(set)
    scanned = 0
    with manifest.open(newline='') as fh:
        for row in csv.DictReader(fh, delimiter='\t'):
            path = root / row['path']
            if path.suffix.lower() not in ('.docx', '.docm', '.dotx', '.dotm'):
                continue
            if not path.exists():
                continue
            scanned += 1
            try:
                zf = zipfile.ZipFile(path)
            except zipfile.BadZipFile:
                continue
            with zf:
                for name in zf.namelist():
                    if not (name.startswith('word/') and name.endswith('.xml')):
                        continue
                    try:
                        tree = ET.fromstring(zf.read(name))
                    except ET.ParseError:
                        continue
                    for anchor in tree.iter(WP + 'anchor'):
                        k, w = drawing_kind(anchor), drawing_wrap(anchor)
                        through = w == 'wrapNone'
                        counts[(k, through)] += 1
                        docs[(k, through)].add(path.name)
                    for shape in tree.iter():
                        if not shape.tag.startswith(V) or shape.tag.split('}')[1] not in (
                                'shape', 'rect', 'oval', 'roundrect', 'line', 'polyline', 'group'):
                            continue
                        style = shape.get('style') or ''
                        if 'position:absolute' not in style.replace(' ', ''):
                            continue
                        k, w = vml_kind(shape), vml_wrap(shape)
                        through = w in ('through', 'none')
                        counts[('vml-' + k, through)] += 1
                        docs[('vml-' + k, through)].add(path.name)

    print(f'DOCX-family documents scanned: {scanned}')
    print(f'{"kind":<22}{"wrap-through":>13}{"count":>8}{"documents":>11}')
    for (k, through), n in sorted(counts.items()):
        print(f'{k:<22}{str(through):>13}{n:>8}{len(docs[(k, through)]):>11}')
    diff = [(k, through) for (k, through) in counts
            if k in ('draw-shape', 'draw-group', 'vml-draw-shape') and not through]
    total = sum(counts[key] for key in diff)
    names = set()
    for key in diff:
        names |= docs[key]
    print()
    print('The two readings differ only on a draw object with no text box that is not wrap-through:')
    print(f'  {total} objects in {len(names)} documents')
    for n in sorted(names):
        print(f'    {n}')


main()
