#!/usr/bin/env python3
"""How many cells of an ODF spreadsheet the importer makes several paragraphs of.

    census.py <corpus-root>

Calc's ODF filter never puts the EditEngine into single-line mode, so a cell becomes one
paragraph per break whatever its wrap option says. Two spellings produce that, and a census that
looks for only the first understates the reach — this one cost round 80 a surprise:

    `breaks`  a cell whose `text:p` holds a raw U+000A or U+000D. LibreOffice's own `.ods`
              export writes a multi-line cell this way and writes no `text:line-break` at all.
    `paras`   a cell with more than one `text:p` child of its own.

Both are counted only for cells that do NOT wrap, because a wrapping cell never widens the print
area (`if (bWidth && bBreak) return 0`, sc/source/core/data/column2.cxx:226) and is measured by a
different branch for height. `wrapbreaks` is the wrapping remainder, for contrast.

A cell holding a `draw:` shape is excluded from `paras` and counted as `shapecells` instead: the
shape's paragraphs are not the cell's, although `OdfContentReader.ReadShape` appends them to it.

`widest-ratio` is the largest ratio of a multi-paragraph cell's whole string to its longest
paragraph — a proxy for how much narrower the reference's print area is than a whole-string
measurement makes it.
"""
import pathlib
import re
import sys
import zipfile

CELL = re.compile(r'<table:table-cell\b([^>]*?)(?:/>|>((?:(?!</table:table-cell>).)*)'
                  r'</table:table-cell>)', re.S)
PARA = re.compile(r'<text:p[^>]*>((?:(?!</text:p>).)*)</text:p>', re.S)
STYLE = re.compile(r'<style:style style:name="([^"]+)"[^>]*style:family="table-cell"'
                   r'((?:(?!</style:style>).)*)(?:</style:style>|/>)', re.S)
TAG = re.compile(r'<[^>]+>')


def wraps(content):
    """Which `table-cell` automatic styles set `fo:wrap-option="wrap"`."""
    return {m.group(1): 'fo:wrap-option="wrap"' in m.group(2) for m in STYLE.finditer(content)}


def census(path):
    try:
        content = zipfile.ZipFile(path).read('content.xml').decode('utf-8', 'replace')
    except Exception:
        return None

    wrap = wraps(content)
    breaks = paras = wrapbreaks = shapecells = 0
    widest = 1.0

    for cell in CELL.finditer(content):
        inner = cell.group(2)
        if not inner:
            continue

        name = re.search(r'table:style-name="([^"]+)"', cell.group(1))
        wrapped = wrap.get(name.group(1), False) if name is not None else False

        if '<draw:' in inner:
            if inner.count('<text:p') > 1:
                shapecells += 1
            continue

        pieces = [TAG.sub('', p) for p in PARA.findall(inner)]
        text = '\n'.join(pieces)
        if '\n' not in text and '\r' not in text:
            continue

        if wrapped:
            wrapbreaks += 1
            continue

        if len(pieces) > 1:
            paras += 1
        else:
            breaks += 1

        longest = max(len(piece) for piece in re.split(r'\r\n|\n|\r', text))
        if longest:
            widest = max(widest, len(text) / longest)

    return breaks, paras, wrapbreaks, shapecells, widest


def main():
    root = pathlib.Path(sys.argv[1])
    rows = []
    for path in sorted(root.rglob('*')):
        if not path.is_file() or path.suffix.lower() not in ('.ods', '.ots', '.fods'):
            continue
        answer = census(path)
        if answer is None or sum(answer[:4]) == 0:
            continue
        rows.append((path.name, *answer))

    rows.sort(key=lambda row: -(row[1] + row[2]))
    print('document\tbreaks\tparas\twrapbreaks\tshapecells\twidest-ratio')
    for name, breaks, paras, wrapbreaks, shapecells, widest in rows:
        print(f'{name}\t{breaks}\t{paras}\t{wrapbreaks}\t{shapecells}\t{widest:.2f}')
    print(f'TOTAL\t{sum(r[1] for r in rows)}\t{sum(r[2] for r in rows)}\t'
          f'{sum(r[3] for r in rows)}\t{sum(r[4] for r in rows)}\t{len(rows)} documents')


if __name__ == '__main__':
    main()
