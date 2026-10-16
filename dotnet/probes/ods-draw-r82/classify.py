#!/usr/bin/env python3
"""Classify the failing rows of an `.ods` sweep by what the document holds.

    classify.py <rows.tsv> <corpus-root>

`rows.tsv` is `sweep-ours.sh`'s output and `corpus-root` the directory its first column is
relative to. Three tests, applied in this order, because they are in decreasing order of how
specifically they explain a divergence:

  shape     the document holds a `draw:` shape anchored in a cell whose `text:p` carry ink.
            Until round 82 those paragraphs were read as the *cell's* text, so they sized its
            row, spilled across its neighbours and widened the used area.
  chart     the document embeds a chart sub-document. A chart's own text is laid out by
            `chart2` and is a different question from a sheet's.
  volatile  the document holds TODAY(), NOW() or RAND(). The reference recalculates on open
            and we print the value the file cached, so the two drift apart with the calendar.
  other     none of the three.

The counts are printed per class and per verdict, and every row is printed with the three
measurements so a reader can disagree with the ordering.
"""
import collections
import os
import re
import sys
import zipfile
from xml.etree import ElementTree as ET

DRAW = '{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}'
TABLE = '{urn:oasis:names:tc:opendocument:xmlns:table:1.0}'
TEXT = '{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'


def shapes(container, out):
    """Every drawing element under a cell, through the two transparent wrappers."""
    for child in container:
        if not child.tag.startswith(DRAW):
            continue
        local = child.tag[len(DRAW):]
        if local in ('g', 'a'):
            shapes(child, out)
        else:
            out.append((local, child))


def measure(path):
    with zipfile.ZipFile(path) as package:
        raw = package.read('content.xml')
        charts = len([n for n in package.namelist() if n.endswith('/content.xml')])

    volatile = len(re.findall(rb'(?:TODAY\(\)|NOW\(\)|RAND\(\))', raw))
    inked = 0
    for cell in ET.fromstring(raw).iter():
        if cell.tag not in (TABLE + 'table-cell', TABLE + 'covered-table-cell'):
            continue
        found = []
        shapes(cell, found)
        for _, element in found:
            text = ''.join(''.join(p.itertext()) for p in element.findall('.//' + TEXT + 'p'))
            if text.strip():
                inked += 1

    return inked, charts, volatile


def main():
    rows_path, root = sys.argv[1], sys.argv[2]
    classes = collections.defaultdict(list)

    print('%-11s %5s %5s %5s  %-9s %-13s %s'
          % ('verdict', 'shape', 'chart', 'volat', 'pages', 'glyphs', 'document'))

    for line in open(rows_path, encoding='utf-8'):
        row = line.rstrip('\n').split('\t')
        if len(row) < 9 or row[6] == 'match':
            continue

        inked, charts, volatile = measure(os.path.join(root, row[0]))
        kind = ('shape' if inked else
                'chart' if charts else
                'volatile' if volatile else 'other')
        classes[kind].append(row[0])
        print('%-11s %5d %5d %5d  %-9s %-13s %s'
              % (row[6], inked, charts, volatile, row[2], row[8], row[0]))

    print()
    for kind, members in sorted(classes.items(), key=lambda item: -len(item[1])):
        print(kind, len(members))


main()
