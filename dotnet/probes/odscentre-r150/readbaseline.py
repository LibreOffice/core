#!/usr/bin/env python3
"""Each arm's baseline, from the PDF's own `Td`, turned the right way up.

`read.py` beside this reports `pymupdf`'s span bbox, whose top is the INK top and sits the face's
ascent above the baseline -- 9.056 pt here. A test comparing its own drawn origin, which is a
baseline, against that number is out by exactly that much, and `dotnet/CLAUDE.md` records a round
nearly filing a font-metric bug on the same confusion.
"""
import re
import sys

import pymupdf

doc = pymupdf.open(sys.argv[1] if len(sys.argv) > 1 else 'fixture-out/sheet-print-centring.pdf')
for i in range(doc.page_count):
    height = doc[i].rect.height
    for m in re.finditer(rb'([-\d.]+)\s+([-\d.]+)\s+Td', doc[i].read_contents()):
        print('page %d  x %8.3f  baseline %8.3f' % (i, float(m.group(1)), height - float(m.group(2))))
