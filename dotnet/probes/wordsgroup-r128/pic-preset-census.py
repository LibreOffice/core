#!/usr/bin/env python3
"""Census: pictures whose own shape properties declare a non-rectangular geometry.

A `pic:pic` carries an `a:prstGeom` or an `a:custGeom` in its `pic:spPr` exactly as a
`wps:wsp` does, and LibreOffice clips the bitmap to it. Counts, per corpus document, how
many pictures state one and which presets they name -- OOXML zip parts only, no rendering.

Usage: pic-preset-census.py <corpus-root> [--ext docx,pptx,xlsx]
"""
import collections
import os
import re
import sys
import zipfile

A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
PIC = 'http://schemas.openxmlformats.org/drawingml/2006/picture'
XDR = 'http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing'
P = 'http://schemas.openxmlformats.org/presentationml/2006/main'

import xml.etree.ElementTree as ET


def parts(z, want):
    for name in z.namelist():
        if not name.endswith('.xml'):
            continue
        if want == 'docx' and not (name.startswith('word/') and
                                   (name.endswith('document.xml') or '/header' in name
                                    or '/footer' in name or name.endswith('endnotes.xml')
                                    or name.endswith('footnotes.xml'))):
            continue
        if want == 'pptx' and not name.startswith('ppt/slides/'):
            continue
        if want == 'xlsx' and not name.startswith('xl/drawings/'):
            continue
        yield name


def scan(path, ext):
    """(pictures with a geometry, presets, custom count, total pictures)."""
    presets = collections.Counter()
    custom = 0
    total = 0
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return presets, custom, total
    with z:
        for name in parts(z, ext):
            try:
                root = ET.fromstring(z.read(name))
            except Exception:
                continue
            for pic in root.iter():
                tag = pic.tag
                if tag not in (f'{{{PIC}}}pic', f'{{{XDR}}}pic', f'{{{P}}}pic'):
                    continue
                total += 1
                sp = None
                for child in pic:
                    if child.tag.split('}')[-1] == 'spPr':
                        sp = child
                if sp is None:
                    continue
                prst = sp.find(f'{{{A}}}prstGeom')
                cust = sp.find(f'{{{A}}}custGeom')
                if cust is not None:
                    custom += 1
                elif prst is not None:
                    name_ = prst.get('prst') or ''
                    if name_ and name_ != 'rect':
                        presets[name_] += 1
    return presets, custom, total


def main():
    root = sys.argv[1]
    exts = ('docx', 'docm', 'pptx', 'pptm', 'xlsx', 'xlsm')
    rows = []
    for base, _dirs, files in os.walk(root):
        for f in sorted(files):
            low = f.lower()
            ext = low.rsplit('.', 1)[-1] if '.' in low else ''
            if ext not in exts:
                continue
            kind = 'docx' if ext.startswith('doc') else ('pptx' if ext.startswith('ppt') else 'xlsx')
            p = os.path.join(base, f)
            presets, custom, total = scan(p, kind)
            n = sum(presets.values()) + custom
            if n:
                rows.append((n, custom, total, kind, f,
                             ','.join(f'{k}:{v}' for k, v in presets.most_common())))
    rows.sort(reverse=True)
    print('shaped\tcustom\tpictures\ttrack\tdocument\tpresets')
    for r in rows:
        print('\t'.join(str(x) for x in r))
    per = collections.Counter(r[3] for r in rows)
    print(f'# documents with a shaped picture: {len(rows)}  ' +
          '  '.join(f'{k}={v}' for k, v in sorted(per.items())))
    allp = collections.Counter()
    for r in rows:
        for item in r[5].split(','):
            if item:
                k, v = item.rsplit(':', 1)
                allp[k] += int(v)
    print('# presets: ' + '  '.join(f'{k}={v}' for k, v in allp.most_common()))


main()
