#!/usr/bin/env python3
"""Do the styles a conditional format applies ever state a font size?

`ScColumn::GetNeededSize` builds the font it measures a cell with from the *conditional
result* first — `pPattern->fillFontOnly(aFont, pDev, &fFontZoom, pCondSet, pTableSet,
nScript)` over `pCondSet = rDocument.GetCondResult(nCol, nRow, nTab)`
(`sc/source/core/data/column2.cxx`:283-292) — and `pCondSet` is the applied style's item
set, searched *through its parent*. So a conditional style that states no font of its own
answers the document default's, and a cell inside a conditional format is measured in that
face rather than in its own.

This counts how often the applied style states a size, which is how much of that rule a
reader can ignore.

Usage: condfont-census.py <root> [...]
"""
import sys
import zipfile
from pathlib import Path
from xml.etree import ElementTree as ET

STYLE = '{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
CALCEXT = '{urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0}'
FO = '{urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0}'


def roots(path):
    if path.suffix.lower() == '.fods':
        yield ET.fromstring(path.read_bytes())
        return
    with zipfile.ZipFile(path) as z:
        for name in ('content.xml', 'styles.xml'):
            try:
                yield ET.fromstring(z.read(name))
            except KeyError:
                pass


def scan(path):
    trees = list(roots(path))
    applied = set()
    for root in trees:
        for style in root.iter(STYLE + 'style'):
            if style.get(STYLE + 'family') != 'table-cell':
                continue
            for child in style:
                if child.tag == STYLE + 'map' and child.get(STYLE + 'apply-style-name'):
                    applied.add(child.get(STYLE + 'apply-style-name'))
        for condition in root.iter(CALCEXT + 'condition'):
            if condition.get(CALCEXT + 'apply-style-name'):
                applied.add(condition.get(CALCEXT + 'apply-style-name'))
    if not applied:
        return 0, 0, 0
    sized = 0
    found = 0
    for root in trees:
        for style in root.iter(STYLE + 'style'):
            name = style.get(STYLE + 'name') or ''
            display = style.get(STYLE + 'display-name') or ''
            if name not in applied and display not in applied:
                continue
            found += 1
            for child in style:
                if child.tag == STYLE + 'text-properties' and child.get(FO + 'font-size'):
                    sized += 1
                    break
    return len(applied), found, sized


def main():
    where = [Path(a) for a in sys.argv[1:]]
    paths = sorted({p.resolve() for root in where for p in root.rglob('*')
                    if p.is_file() and p.suffix.lower() in ('.ods', '.fods', '.ots')})
    print('document\tapplied_names\tstyles_found\tstyles_stating_a_size')
    documents = totals = sized = 0
    for p in paths:
        try:
            names, found, with_size = scan(p)
        except Exception as exc:                    # noqa: BLE001
            print(f'{p.name}\tERROR\t{exc}\t')
            continue
        if names == 0:
            continue
        documents += 1
        totals += found
        sized += with_size
        print(f'{p.name}\t{names}\t{found}\t{with_size}')
    print(f'# {documents} documents apply a conditional style; {totals} such styles found, '
          f'{sized} state a font size', file=sys.stderr)


if __name__ == '__main__':
    main()
