#!/usr/bin/env python3
"""Exact text baselines from a PDF's content streams.

pdftotext -bbox reports the *ink* box, which moves with whichever glyphs a line happens to hold,
so it cannot settle a one-twip question. The Td/TD/Tm operators carry the pen itself.
"""
import re
import sys
import zlib


def streams(data):
    for m in re.finditer(rb'stream\r?\n', data):
        start = m.end()
        end = data.find(b'endstream', start)
        if end < 0:
            continue
        raw = data[start:end]
        try:
            yield zlib.decompress(raw)
        except zlib.error:
            yield raw


def baselines(path):
    data = open(path, 'rb').read()
    out = []
    for body in streams(data):
        text = body.decode('latin-1')
        page = []
        tm = None
        for m in re.finditer(
                r'BT|ET|([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+Tm'
                r'|([-\d.]+)\s+([-\d.]+)\s+(?:Td|TD)'
                r'|\((?:[^()\\]|\\.)*\)\s*T[jJ]|\][\s]*TJ', text):
            g = m.group(0)
            if g.startswith('BT'):
                tm = [0.0, 0.0]
            elif m.group(5) is not None:
                tm = [float(m.group(5)), float(m.group(6))]
            elif m.group(7) is not None and tm is not None:
                tm[0] += float(m.group(7))
                tm[1] += float(m.group(8))
            elif tm is not None and (g.endswith('Tj') or g.endswith('TJ')):
                page.append(round(tm[1], 4))
        if page:
            out.append(page)
    return out


if __name__ == '__main__':
    for i, page in enumerate(baselines(sys.argv[1]), 1):
        ys = sorted(set(page), reverse=True)
        print(f'--- page {i}: {len(ys)} baselines')
        prev = None
        for y in ys:
            d = '' if prev is None else f'{prev - y:8.4f}'
            print(f'  y={y:9.4f} dy={d}')
            prev = y
