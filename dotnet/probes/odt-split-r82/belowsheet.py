#!/usr/bin/env python3
"""How many glyphs a rendering draws below its own sheet, per document.

The companion to `tdrange.py`: the range says a defect is there, this says how much of the
document it is.  Glyphs rather than characters, counted off the hex strings the text-showing
operators carry, because that is what survives clipping-blind instruments like `pdftotext`.
"""
import pathlib
import re
import sys

import pymupdf

TEXTOBJ = re.compile(rb'BT\b(.*?)\bET\b', re.S)
TD = re.compile(rb'([-\d.]+)\s+([-\d.]+)\s+T[dD]\b')
TM = re.compile(rb'([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+Tm')
SHOW = re.compile(rb'<([0-9A-Fa-f\s]*)>\s*Tj|\[(.*?)\]\s*TJ', re.S)
HEX = re.compile(rb'<([0-9A-Fa-f\s]*)>')


def glyphs(body):
    total = 0
    for m in SHOW.finditer(body):
        if m.group(1) is not None:
            total += len(re.sub(rb'\s', b'', m.group(1))) // 4
        else:
            for h in HEX.finditer(m.group(2)):
                total += len(re.sub(rb'\s', b'', h.group(1))) // 4
    return total


def below(pdf):
    doc = pymupdf.open(pdf)
    off = on = 0
    try:
        for page in doc:
            stream = b''.join(doc.xref_stream(x) for x in page.get_contents())
            for obj in TEXTOBJ.finditer(stream):
                body = obj.group(1)
                ys = [float(m.group(2)) for m in TD.finditer(body)]
                ys += [float(m.group(6)) for m in TM.finditer(body)]
                n = glyphs(body)
                if ys and min(ys) < 0:
                    off += n
                else:
                    on += n
    finally:
        doc.close()
    return on, off


if __name__ == '__main__':
    root = pathlib.Path(sys.argv[1])
    print('document\tonsheet\tbelowsheet')
    for pdf in sorted(root.glob('*.pdf')):
        on, off = below(pdf)
        if off:
            print(f'{pdf.stem}\t{on}\t{off}')
