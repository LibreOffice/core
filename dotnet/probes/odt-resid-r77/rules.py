#!/usr/bin/env python3
"""The x positions of a page's vertical rules, which are a table's column boundaries.

A cell border is drawn as a stroked line or a filled rectangle; both are read here, so the
column grid of a rendering can be compared against another's without reading its text.

usage: rules.py <pdf> <page>
"""
import re
import subprocess
import sys
import zlib


def page_streams(pdf: str, page: int):
    """Every content stream of the file, inflated. Page selection is by order of appearance."""
    data = open(pdf, 'rb').read()
    out = []
    for m in re.finditer(rb'stream\r?\n', data):
        start = m.end()
        end = data.find(b'endstream', start)
        try:
            out.append(zlib.decompress(data[start:end]))
        except zlib.error:
            continue
    return out


def verticals(text: str):
    xs = set()
    for m in re.finditer(r'([-\d.]+) ([-\d.]+) m\s+([-\d.]+) ([-\d.]+) l', text):
        x0, y0, x1, y1 = (float(g) for g in m.groups())
        if abs(x0 - x1) < 0.2 and abs(y0 - y1) > 2:
            xs.add(round(x0, 1))
    for m in re.finditer(r'([-\d.]+) ([-\d.]+) ([-\d.]+) ([-\d.]+) re', text):
        x, y, w, h = (float(g) for g in m.groups())
        if abs(w) < 2 and abs(h) > 2:
            xs.add(round(x, 1))
    return sorted(xs)


if __name__ == '__main__':
    pdf, page = sys.argv[1], int(sys.argv[2])
    streams = [s for s in page_streams(pdf, page) if b're' in s or b' l' in s]
    if page - 1 < len(streams):
        print(' '.join(f'{x:.1f}' for x in verticals(streams[page - 1].decode('latin1'))))
