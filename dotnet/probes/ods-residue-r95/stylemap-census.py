#!/usr/bin/env python3
"""Which `style:map` elements are a *conditional cell format* and which are a number format.

ODF 1.2 states a conditional cell format as a `<style:map>` on a `<style:style
style:family="table-cell">`, carrying `style:condition`, `style:apply-style-name` and
`style:base-cell-address` (OpenDocument 1.2 part 1, 16.3 and 20.360). The identical
element name is *also* how a `number:*-style` states its positive/negative/zero
sub-formats, and there it carries no base cell address and its parent is a number style.

Counting the element without its parent therefore counts number formats, which is what
`probes/ods-notes-r92/results.md` §2's "54 state a `style:map` in content.xml" did.

Usage: stylemap-census.py <root> [...]
"""
import sys
import zipfile
from pathlib import Path
from xml.etree import ElementTree as ET

STYLE = '{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
CALCEXT = '{urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0}'


def parts(path):
    if path.suffix.lower() == '.fods':
        yield 'content.xml', path.read_bytes()
        return
    with zipfile.ZipFile(path) as z:
        for name in ('content.xml', 'styles.xml'):
            try:
                yield name, z.read(name)
            except KeyError:
                pass


def scan(path):
    cell_maps = 0
    number_maps = 0
    other_maps = 0
    calcext = 0
    for _, data in parts(path):
        root = ET.fromstring(data)
        for parent in root.iter():
            family = parent.get(STYLE + 'family')
            for child in parent:
                if child.tag != STYLE + 'map':
                    continue
                if parent.tag == STYLE + 'style' and family == 'table-cell':
                    cell_maps += 1
                elif parent.tag.startswith(
                        '{urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0}'):
                    number_maps += 1
                else:
                    other_maps += 1
        calcext += sum(1 for e in root.iter(CALCEXT + 'conditional-format'))
    return cell_maps, number_maps, other_maps, calcext


def main():
    roots = [Path(a) for a in sys.argv[1:]]
    paths = sorted({p.resolve() for root in roots for p in root.rglob('*')
                    if p.is_file() and p.suffix.lower() in ('.ods', '.fods', '.ots')})
    print('document\tcell_maps\tnumber_maps\tother_maps\tcalcext')
    both = only_map = only_calcext = neither = 0
    for p in paths:
        try:
            c, n, o, x = scan(p)
        except Exception as exc:                    # noqa: BLE001
            print(f'{p.name}\tERROR\t{exc}\t\t')
            continue
        print(f'{p.name}\t{c}\t{n}\t{o}\t{x}')
        if c and x:
            both += 1
        elif c:
            only_map += 1
        elif x:
            only_calcext += 1
        else:
            neither += 1
    print(f'# {len(paths)} documents: {both} state both spellings, '
          f'{only_map} a table-cell style:map alone, '
          f'{only_calcext} calcext alone, {neither} neither', file=sys.stderr)


if __name__ == '__main__':
    main()
