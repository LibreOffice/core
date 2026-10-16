#!/usr/bin/env python3
"""How far the ODF 200-row rule reaches across a corpus of spreadsheets.

    rowlimit-census.py <corpus-root>

A row block is excluded from Calc's height recalculation when its last row is past row 200 and its
style states both `style:row-height` and `style:use-optimal-row-height="true"`
(sc/source/filter/xml/xmlrowi.cxx:215-244). This walks each sheet's `table:table-row` elements,
counts the rows that meet that test, and reports how many rows a document keeps rather than
recomputes.

Reported per document: `kept` rows past the limit, `total` rows the file states a block for, and
the distinct stated heights among the kept ones — a document whose kept heights are all the same
as its recomputed one will not move a page whatever the rule says.
"""
import pathlib
import re
import sys
import zipfile

LIMIT = 200

ROW = re.compile(r'<table:table-row\b[^>]*?(?:/>|>)')
TABLE = re.compile(r'<table:table\b[^>]*?(?:/>|>)')
STYLE = re.compile(r'<style:style style:name="([^"]+)"[^>]*style:family="table-row"'
                   r'((?:(?!</style:style>).)*)(?:</style:style>|/>)', re.S)


def row_styles(content):
    """Each `table-row` automatic style as (stated height, optimal flag)."""
    answer = {}
    for match in STYLE.finditer(content):
        height = re.search(r'style:row-height="([^"]*)"', match.group(2))
        optimal = 'style:use-optimal-row-height="true"' in match.group(2)
        answer[match.group(1)] = (height.group(1) if height else None, optimal)
    return answer


def census(path):
    try:
        content = zipfile.ZipFile(path).read('content.xml').decode('utf-8', 'replace')
    except Exception:
        return None

    styles = row_styles(content)
    kept = total = 0
    heights = set()
    row = 0

    # `<table:table\b` also matches `table-row`, `table-cell` and `table-column`, so the sheet
    # element has to be matched with an explicit lookahead or every cell resets the row counter.
    for match in re.finditer(r'<table:table(?=[ />])[^>]*?(?:/>|>)|' + ROW.pattern, content):
        token = match.group(0)
        if token.startswith('<table:table-row'):
            repeat = re.search(r'number-rows-repeated="(\d+)"', token)
            repeat = int(repeat.group(1)) if repeat else 1
            name = re.search(r'table:style-name="([^"]+)"', token)
            height, optimal = styles.get(name.group(1), (None, False)) if name else (None, False)
            row += repeat
            # Only the blocks a real sheet states; the millionth padding row is not one.
            if repeat < 100000:
                total += repeat
                if height is not None and optimal and row - 1 > LIMIT:
                    kept += repeat
                    heights.add(height)
        else:
            row = 0

    return kept, total, heights


def main():
    root = pathlib.Path(sys.argv[1])
    rows = []
    for path in sorted(root.rglob('*')):
        if not path.is_file() or path.suffix.lower() not in ('.ods', '.ots', '.fods'):
            continue
        answer = census(path)
        if answer is None or answer[0] == 0:
            continue
        rows.append((path.name, *answer))

    rows.sort(key=lambda row: -row[1])
    print('document\tkept\tstated\theights')
    for name, kept, total, heights in rows:
        print(f'{name}\t{kept}\t{total}\t{",".join(sorted(heights))}')
    print(f'TOTAL\t{sum(r[1] for r in rows)}\t{sum(r[2] for r in rows)}\t'
          f'{len(rows)} documents')


if __name__ == '__main__':
    main()
