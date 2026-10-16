#!/usr/bin/env python3
"""Census the reach of O96 over the converted .ods corpus, by what it PAINTS.

Three figures, each narrower than the last, because a count of styles is not a
count of cells and a count of cells is not a count of cells the fix moves:

  1. documents holding a `number:*-style` with a `style:map` at all;
  2. documents where a CELL names such a style;
  3. documents holding a cell whose VALUE selects one of the mapped sections
     rather than the named style's own body -- which is the set this change can
     move, and the figure to quote.

A fourth column counts the cells whose selected section states one of the ten
keyword colours, since that half is invisible to every gate column.
"""
import re, sys, zipfile, pathlib, collections
from xml.etree import ElementTree as ET

N = 'urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0'
S = 'urn:oasis:names:tc:opendocument:xmlns:style:1.0'
F = 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'
T = 'urn:oasis:names:tc:opendocument:xmlns:table:1.0'
O = 'urn:oasis:names:tc:opendocument:xmlns:office:1.0'

TEN = {'#000000', '#0000ff', '#00ff00', '#00ffff', '#ff0000',
       '#ff00ff', '#808000', '#808080', '#ffff00', '#ffffff'}


def matches(comparison, operand, value):
    return {'<': value < operand, '<=': value <= operand, '>': value > operand,
            '>=': value >= operand, '<>': value != operand}.get(comparison, value == operand)


def condition_of(text):
    if not text.startswith('value()'):
        return None
    rest = text[len('value()'):].replace('!=', '<>')
    m = re.match(r'^(<=|>=|<>|<|>|=)\s*(-?[0-9.]+)$', rest.strip())
    return (m.group(1), float(m.group(2))) if m else None


def styles_of(zf):
    """Every data style and every cell style in the package, by name."""
    data, cell = {}, {}
    for part in ('styles.xml', 'content.xml'):
        try:
            tree = ET.fromstring(zf.read(part))
        except Exception:
            continue
        for el in tree.iter():
            if el.tag.startswith('{' + N + '}') and el.tag.endswith('-style'):
                name = el.get(f'{{{S}}}name')
                if name:
                    data[name] = el
            elif el.tag == f'{{{S}}}style' and el.get(f'{{{S}}}family') == 'table-cell':
                name = el.get(f'{{{S}}}name')
                if name:
                    cell[name] = el
    return data, cell


def data_style_of(name, cell):
    """The data style a cell style names, following its parent chain."""
    for _ in range(16):
        style = cell.get(name)
        if style is None:
            return None
        named = style.get(f'{{{S}}}data-style-name')
        if named:
            return named
        name = style.get(f'{{{S}}}parent-style-name')
        if not name:
            return None
    return None


def colour_of(style):
    props = style.find(f'{{{S}}}text-properties')
    value = props.get(f'{{{F}}}color') if props is not None else None
    return value.lower() if value and value.lower() in TEN else None


def main():
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/corpus-odf/ods')
    files = sorted(root.glob('*.ods'))

    rows = []
    for path in files:
        try:
            zf = zipfile.ZipFile(path)
        except Exception:
            continue
        data, cell = styles_of(zf)

        mapped = {n: el for n, el in data.items() if el.find(f'{{{S}}}map') is not None}
        named_by_cell = 0
        selecting = 0
        coloured = 0

        if mapped:
            try:
                content = ET.fromstring(zf.read('content.xml'))
            except Exception:
                content = None

            if content is not None:
                for c in content.iter(f'{{{T}}}table-cell'):
                    style_name = c.get(f'{{{T}}}style-name')
                    if style_name is None:
                        continue
                    ds = data_style_of(style_name, cell)
                    if ds is None or ds not in mapped:
                        continue
                    repeat = int(c.get(f'{{{T}}}number-columns-repeated') or 1)
                    repeat = min(repeat, 1024)
                    named_by_cell += repeat

                    raw = c.get(f'{{{O}}}value')
                    if raw is None:
                        continue
                    try:
                        value = float(raw)
                    except ValueError:
                        continue

                    owner = mapped[ds]
                    chosen = owner
                    for m in owner.findall(f'{{{S}}}map'):
                        parsed = condition_of(m.get(f'{{{S}}}condition') or '')
                        applied = m.get(f'{{{S}}}apply-style-name')
                        if parsed and applied in data and matches(parsed[0], parsed[1], value):
                            chosen = data[applied]
                            break

                    if chosen is not owner:
                        selecting += repeat
                    if colour_of(chosen):
                        coloured += repeat

        rows.append((path.name, len(mapped), named_by_cell, selecting, coloured))

    print('document\tmapped_styles\tcells_naming_one\tcells_selecting_a_mapped_section\tcells_taking_a_keyword_colour')
    for r in rows:
        if r[1]:
            print('\t'.join(str(x) for x in r))

    docs = len(rows)
    with_styles = sum(1 for r in rows if r[1])
    with_cells = sum(1 for r in rows if r[2])
    with_select = sum(1 for r in rows if r[3])
    with_colour = sum(1 for r in rows if r[4])
    print(f'# {docs} documents')
    print(f'# {with_styles} hold a number style with a style:map')
    print(f'# {with_cells} have a CELL naming one ({sum(r[2] for r in rows)} cells)')
    print(f'# {with_select} hold a cell whose VALUE selects a mapped section '
          f'({sum(r[3] for r in rows)} cells)')
    print(f'# {with_colour} hold a cell taking one of the ten keyword colours '
          f'({sum(r[4] for r in rows)} cells)')


if __name__ == '__main__':
    main()
