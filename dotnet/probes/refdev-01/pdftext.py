#!/usr/bin/env python3
"""Every text-showing operator's page, baseline and font size, at full stream precision.

`pdf-ops.py dump` rounds its coordinates to two decimals, which is 0.7 of a hundredth of a
millimetre — not enough to settle a one-unit question on a device whose logical unit *is* the
hundredth of a millimetre. LibreOffice writes three decimals, so the numbers are there; this
reads them without rounding.

Only what the probes need: `BT`/`ET`, `Tm`, `Td`, `TD`, `T*`, `TL`, `Tf`, and the four show
operators. No clipping, no XObjects, no rotation — the probe documents contain none.

    pdftext.py <pdf>   ->   TSV: page  x  y  size  fontname  nglyphs
"""
import re
import sys
import zlib


def pages(data):
    """Each page's content bytes and MediaBox, in document order.

    The probe PDFs are written by LibreOffice with one uncompressed cross-reference-free
    layout per page, so walking the object table and following /Kids is more machinery than
    is needed: the page objects appear in order, and each names exactly one content stream.
    """
    objs = {}
    for m in re.finditer(rb'(\d+)\s+(\d+)\s+obj\b', data):
        objs[int(m.group(1))] = m.end()

    def body(num):
        s = objs[num]
        e = data.find(b'endobj', s)
        return data[s:e]

    def stream(num):
        b = body(num)
        m = re.search(rb'stream\r?\n', b)
        if not m:
            return b''
        raw = b[m.end():b.rfind(b'endstream')]
        if b'/FlateDecode' in b[:m.start()]:
            try:
                return zlib.decompress(raw)
            except zlib.error:
                return b''
        return raw

    out = []
    for num, off in sorted(objs.items()):
        b = body(num)
        if b'/Type' not in b or b'/Page' not in b:
            continue
        if re.search(rb'/Type\s*/Page\b', b) is None:
            continue
        mb = re.search(rb'/MediaBox\s*\[\s*([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)', b)
        box = tuple(float(x) for x in mb.groups()) if mb else (0, 0, 612, 792)
        cm = re.search(rb'/Contents\s+(\d+)\s+\d+\s+R', b)
        if not cm:
            continue
        out.append((stream(int(cm.group(1))), box))
    return out


NUM = rb'[-+]?[\d.]+'
TOK = re.compile(
    rb'(?P<n>' + NUM + rb')|'
    rb'/(?P<name>[^\s/\[\]<>()]+)|'
    rb'(?P<str><[0-9A-Fa-f]*>)|'
    rb'(?P<op>BT|ET|Tm|TD|Td|T\*|TL|Tf|TJ|Tj|\'|")')


def show(stream_bytes):
    """(x, y, size, font, glyphs) for every show operator, in stream order."""
    st = []
    out = []
    tm = None
    line = None
    lead = 0.0
    size = 0.0
    font = ''
    nglyph = 0
    for m in TOK.finditer(stream_bytes):
        if m.lastgroup == 'n':
            st.append(float(m.group('n')))
            continue
        if m.lastgroup == 'name':
            st.append(m.group('name').decode('latin1'))
            continue
        if m.lastgroup == 'str':
            nglyph += max(0, (len(m.group('str')) - 2) // 2)
            continue
        op = m.group('op')
        if op == b'BT':
            tm = line = (0.0, 0.0)
            nglyph = 0
        elif op == b'ET':
            tm = line = None
        elif op == b'Tm' and len(st) >= 6:
            tm = line = (st[-2], st[-1])
        elif op in (b'Td', b'TD') and len(st) >= 2 and line is not None:
            if op == b'TD':
                lead = -st[-1]
            tm = line = (line[0] + st[-2], line[1] + st[-1])
        elif op == b'T*' and line is not None:
            tm = line = (line[0], line[1] - lead)
        elif op == b'TL' and st:
            lead = st[-1]
        elif op == b'Tf' and len(st) >= 2:
            font, size = st[-2], st[-1]
        elif op in (b'TJ', b'Tj', b"'", b'"'):
            if op in (b"'", b'"') and line is not None:
                tm = line = (line[0], line[1] - lead)
            if tm is not None:
                out.append((tm[0], tm[1], size, font, nglyph))
            nglyph = 0
        st.clear()
    return out


def rows(path):
    """(page, x, y_from_top_pt, size, font, glyphs), y measured downwards from the page top."""
    got = []
    for i, (content, box) in enumerate(pages(open(path, 'rb').read()), start=1):
        top = box[3]
        for x, y, size, font, n in show(content):
            got.append((i, x, top - y, size, font, n))
    return got


if __name__ == '__main__':
    for r in rows(sys.argv[1]):
        print('%d\t%.4f\t%.4f\t%.4f\t%s\t%d' % r)
