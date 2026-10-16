#!/usr/bin/env python3
"""Does the `calcext:` spelling of a conditional format cover every cell the ODF 1.2
`style:map` spelling reaches?

For each document: find every cell style (named or automatic) carrying a `style:map`
with a `style:base-cell-address` — ODF 1.2's conditional cell format — then walk every
table and record the address of each cell whose *effective* style is one of them
(the cell's own `table:style-name`, else the row's `table:default-cell-style-name`,
else the column's). Finally check each such address against the union of the
`calcext:conditional-format/@calcext:target-range-address` ranges.

A cell inside a calcext range is one this tree already measures, because
`OdsConditionalFormats` reads that spelling. A cell outside one is what reading
`style:map` would buy.

Usage: stylemap-cover.py <root> [...]
"""
import re
import sys
import zipfile
from pathlib import Path
from xml.etree import ElementTree as ET

OFFICE = '{urn:oasis:names:tc:opendocument:xmlns:office:1.0}'
STYLE = '{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
TABLE = '{urn:oasis:names:tc:opendocument:xmlns:table:1.0}'
CALCEXT = '{urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0}'

CELL = re.compile(r"^(?:\$?(?:'([^']*)'|([^.$']*))\.)?\$?([A-Za-z]+)\$?(\d+)$")


def column_index(letters):
    n = 0
    for ch in letters.upper():
        n = n * 26 + (ord(ch) - 64)
    return n - 1


def parse_range(text):
    """A `Sheet.A1:Sheet.B2` or `Sheet.A1` in OOO syntax, as (sheet, c0, r0, c1, r1)."""
    halves = text.split(':')
    first = CELL.match(halves[0].strip())
    if not first:
        return None
    last = CELL.match(halves[-1].strip()) if len(halves) > 1 else first
    if not last:
        return None
    sheet = first.group(1) or first.group(2) or ''
    return (sheet,
            column_index(first.group(3)), int(first.group(4)) - 1,
            column_index(last.group(3)), int(last.group(4)) - 1)


def styles_with_maps(roots):
    named = set()
    for root in roots:
        for style in root.iter(STYLE + 'style'):
            if style.get(STYLE + 'family') != 'table-cell':
                continue
            for child in style:
                if child.tag == STYLE + 'map' and child.get(STYLE + 'base-cell-address'):
                    named.add(style.get(STYLE + 'name'))
                    break
    return named


def split_list(stated, separator=' '):
    """Split on a separator that is not inside a quoted sheet name.

    A sheet whose name holds a space -- `'Idea planner'.F9:'Idea planner'.F12` -- is
    torn in three by a plain `str.split`, which is what made the first cut of this
    report every mapped cell as uncovered. `SheetAddress.SplitList` is the tree's own
    form of the same rule.
    """
    out = []
    current = []
    quoted = False
    for ch in stated:
        if ch == "'":
            quoted = not quoted
            current.append(ch)
        elif ch == separator and not quoted:
            if current:
                out.append(''.join(current))
                current = []
        else:
            current.append(ch)
    if current:
        out.append(''.join(current))
    return out


def calcext_ranges(root):
    out = []
    for fmt in root.iter(CALCEXT + 'conditional-format'):
        stated = fmt.get(CALCEXT + 'target-range-address') or ''
        for part in split_list(stated):
            if not part.strip():
                continue
            parsed = parse_range(part)
            if parsed:
                out.append(parsed)
    return out


def repeat(element, attribute, cap):
    try:
        return min(int(element.get(TABLE + attribute, '1')), cap)
    except ValueError:
        return 1


def mapped_cells(root, marked, limit=200000):
    """Addresses of cells whose effective style carries a map, capped per document."""
    found = []
    for table in root.iter(TABLE + 'table'):
        sheet = table.get(TABLE + 'name') or ''
        column_styles = []
        for column in table.iter(TABLE + 'table-column'):
            for _ in range(repeat(column, 'number-columns-repeated', 1024)):
                column_styles.append(column.get(TABLE + 'default-cell-style-name'))
        row_index = 0
        for row in table.iter(TABLE + 'table-row'):
            rows = repeat(row, 'number-rows-repeated', 1 << 20)
            row_style = row.get(TABLE + 'default-cell-style-name')
            column_index_ = 0
            for cell in row:
                if cell.tag not in (TABLE + 'table-cell', TABLE + 'covered-table-cell'):
                    continue
                span = repeat(cell, 'number-columns-repeated', 1024)
                own = cell.get(TABLE + 'style-name') or row_style
                for offset in range(span):
                    effective = own
                    if effective is None:
                        at = column_index_ + offset
                        effective = column_styles[at] if at < len(column_styles) else None
                    if effective in marked:
                        for r in range(min(rows, 1024)):
                            found.append((sheet, column_index_ + offset, row_index + r))
                            if len(found) > limit:
                                return found
                column_index_ += span
            row_index += rows
    return found


def covered(address, ranges):
    sheet, column, row = address
    for (name, c0, r0, c1, r1) in ranges:
        if name and sheet and name != sheet:
            continue
        if c0 <= column <= c1 and r0 <= row <= r1:
            return True
    return False


def scan(path):
    roots = []
    if path.suffix.lower() == '.fods':
        roots.append(ET.fromstring(path.read_bytes()))
        content = roots[0]
    else:
        with zipfile.ZipFile(path) as z:
            content = ET.fromstring(z.read('content.xml'))
            roots.append(content)
            try:
                roots.append(ET.fromstring(z.read('styles.xml')))
            except KeyError:
                pass
    marked = styles_with_maps(roots)
    if not marked:
        return 0, 0, 0
    ranges = calcext_ranges(content)
    cells = mapped_cells(content, marked)
    outside = sum(1 for c in cells if not covered(c, ranges))
    return len(marked), len(cells), outside


def main():
    roots = [Path(a) for a in sys.argv[1:]]
    paths = sorted({p.resolve() for root in roots for p in root.rglob('*')
                    if p.is_file() and p.suffix.lower() in ('.ods', '.fods', '.ots')})
    print('document\tmapped_styles\tmapped_cells\tcells_outside_calcext')
    documents = uncovered = 0
    for p in paths:
        try:
            styles, cells, outside = scan(p)
        except Exception as exc:                    # noqa: BLE001
            print(f'{p.name}\tERROR\t{exc}\t')
            continue
        if styles == 0:
            continue
        documents += 1
        if outside:
            uncovered += 1
        print(f'{p.name}\t{styles}\t{cells}\t{outside}')
    print(f'# {documents} documents carry a conditional style:map; '
          f'{uncovered} hold a mapped cell outside every calcext range', file=sys.stderr)


if __name__ == '__main__':
    main()
