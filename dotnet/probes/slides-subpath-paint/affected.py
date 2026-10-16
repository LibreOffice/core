#!/usr/bin/env python3
"""List the slides-track documents a subpath's own fill, stroke or shade can reach.

A document is affected when any part it draws from names a preset whose table entry carries a
subpath that is not `fill="norm" stroke="true"`, or states an `a:path` of its own with `fill="none"`
or `stroke="false"`. This is a census of what a document *declares* and therefore an upper bound —
a preset named in a layout the deck never uses still counts. It is used to pick which documents to
render twice, not to state the reach; the reach is what the two renders actually differ on.

Binary `.ppt` cannot be censused this way, so every one is listed: an Escher shape type maps onto a
preset name at layout time and the type is not a string in the file.

    affected.py <corpus-root> [--controls N]
"""
import argparse
import os
import pathlib
import re
import sys
import zipfile

HERE = pathlib.Path(__file__).resolve().parent
TABLE = HERE.parents[1] / 'src/XRay.Office.Ooxml/DrawingML/PresetShapeGeometry.txt'
OOXML = ('.pptx', '.pptm', '.potx', '.ppsx', '.ppsm', '.potm')
BINARY = ('.ppt', '.pot', '.pps')
FLAT = ('.odp', '.otp', '.fodp')


def presets_with_an_opinionated_subpath():
    names, current, out = {}, None, set()
    for line in TABLE.read_text().splitlines():
        if line.startswith('s '):
            current = line[2:].strip()
            names[current] = []
        elif line.startswith('p ') and current:
            fields = line.split()
            names[current].append((fields[3], fields[4]))
    for name, paths in names.items():
        if any(fill != '-' or stroke != '-' for fill, stroke in paths):
            out.add(name)
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('root')
    parser.add_argument('--controls', type=int, default=25)
    args = parser.parse_args()

    interesting = presets_with_an_opinionated_subpath()
    affected, controls = [], []
    root = pathlib.Path(args.root)

    for path in sorted((root / 'slides').rglob('*')):
        if not path.is_file():
            continue
        suffix = path.suffix.lower()
        if suffix in BINARY:
            affected.append(path)
            continue
        if suffix in FLAT:
            text = path.read_text(errors='replace') if suffix == '.fodp' else ''
            hit = bool(text) and 'draw:enhanced-geometry' in text
            (affected if hit else controls).append(path)
            continue
        if suffix not in OOXML:
            continue
        hit = False
        try:
            package = zipfile.ZipFile(path)
        except Exception:
            continue
        for entry in package.namelist():
            if not entry.endswith('.xml') or '/ppt/' not in '/' + entry:
                continue
            try:
                body = package.read(entry).decode('utf-8', 'replace')
            except Exception:
                continue
            if any(m.group(1) in interesting for m in re.finditer(r'prst="([A-Za-z0-9]+)"', body)):
                hit = True
                break
            if re.search(r'<a:path[^>]*(?:fill="none"|stroke="(?:0|false)")', body):
                hit = True
                break
        (affected if hit else controls).append(path)

    for path in affected:
        print(path)
    for path in controls[:: max(1, len(controls) // max(1, args.controls))][:args.controls]:
        print(path)
    print(f'{len(affected)} affected, {args.controls} controls', file=sys.stderr)


if __name__ == '__main__':
    main()
