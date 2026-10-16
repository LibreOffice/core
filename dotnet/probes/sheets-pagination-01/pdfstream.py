#!/usr/bin/env python3
"""Minimal PDF content-stream extractor: inflates every FlateDecode stream in a file.

Used to read drawing operators (`re`, `f`, `Do`) out of a PDF directly, because a
downscaled raster cannot prove an operator is absent and a bounding box cannot give
a rectangle's exact edges.
"""
import re
import sys
import zlib


def streams(path):
    data = open(path, 'rb').read()
    out = []
    for m in re.finditer(rb'stream\r?\n', data):
        start = m.end()
        end = data.find(b'endstream', start)
        if end < 0:
            continue
        blob = data[start:end]
        try:
            out.append(zlib.decompress(blob))
        except Exception:
            try:
                out.append(zlib.decompressobj().decompress(blob))
            except Exception:
                out.append(blob)
    return out


def content(path):
    return b'\n'.join(streams(path)).decode('latin1')


if __name__ == '__main__':
    sys.stdout.write(content(sys.argv[1]))
