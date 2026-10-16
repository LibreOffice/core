#!/usr/bin/env python3
"""Census the ODF spreadsheet column for the three things round 77 read.

    census.py <corpus-root> [newline|frames|transform]

`newline`   -- table cells whose `text:p` holds a raw U+000A or U+000D, which Calc reads as a
               paragraph break and which the ODF-namespace `text:line-break` never appears for.
`frames`    -- `draw:frame` by how it is wrapped: the cell's own child, inside a `draw:g`, or
               inside a `draw:a`.
`transform` -- shapes stating `draw:transform` rather than `svg:x`/`svg:y`.

Counts are per document and in total; the totals are what the write-up quotes.
"""
import collections
import pathlib
import re
import sys
import zipfile
from xml.etree import ElementTree as ET

DRAW = '{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}'
TABLE = '{urn:oasis:names:tc:opendocument:xmlns:table:1.0}'
PARA = re.compile(r'<text:p[^>]*>((?:(?!</text:p>).)*)</text:p>', re.S)


def content(path):
    try:
        return zipfile.ZipFile(path).read('content.xml')
    except Exception:
        return None


def newline(data, counter):
    text = data.decode('utf-8', 'replace')
    for match in PARA.finditer(text):
        if '\n' in match.group(1) or '\r' in match.group(1):
            counter['paragraphs'] += 1
    counter['line-break'] += text.count('<text:line-break/>')


def frames(data, counter):
    try:
        root = ET.fromstring(data)
    except ET.ParseError:
        return
    for cell in root.iter():
        if cell.tag not in (TABLE + 'table-cell', TABLE + 'covered-table-cell',
                            TABLE + 'shapes'):
            continue
        for child in cell:
            if child.tag == DRAW + 'frame':
                counter['cell-child'] += 1
            elif child.tag == DRAW + 'g':
                counter['group'] += 1
            elif child.tag == DRAW + 'a':
                counter['anchor'] += 1
        for group in cell.iter(DRAW + 'g'):
            counter['in-group'] += sum(1 for _ in group.iter(DRAW + 'frame'))
        for link in cell.iter(DRAW + 'a'):
            counter['in-anchor'] += sum(1 for _ in link.iter(DRAW + 'frame'))


def transform(data, counter):
    text = data.decode('utf-8', 'replace')
    for element in re.finditer(r'<draw:[a-z-]+\b[^>]*>', text):
        if 'draw:transform' in element.group(0):
            counter['transform'] += 1
            if element.group(0).startswith('<draw:frame'):
                counter['transform-frame'] += 1


def main():
    root = pathlib.Path(sys.argv[1])
    what = sys.argv[2] if len(sys.argv) > 2 else 'newline'
    handler = {'newline': newline, 'frames': frames, 'transform': transform}[what]

    total = collections.Counter()
    documents = 0
    for path in sorted(root.rglob('*.ods')):
        data = content(path)
        if data is None:
            continue
        counter = collections.Counter()
        handler(data, counter)
        if counter:
            documents += 1
            print(path.name, dict(counter))
            total.update(counter)
    print('---', documents, 'documents', dict(total))


if __name__ == '__main__':
    main()
