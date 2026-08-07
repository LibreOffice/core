#!/usr/bin/env python3
"""Dump the text-showing operators of one PDF page: font, size, matrix, pen.

Poppler's `-bbox` gives glyph boxes; it does not give the /Tf size, and a font size is
what settles an autofit question. Nothing but poppler is installed here, so this walks
the file directly: it scans every `obj ... endobj`, inflates stream bodies, finds the
page objects in order, and replays the text operators of the one asked for.
"""
import re, sys, zlib


def objects(d):
    out = {}
    for m in re.finditer(rb'(\d+)\s+(\d+)\s+obj\b', d):
        num = int(m.group(1))
        start = m.end()
        end = d.find(b'endobj', start)
        if end < 0:
            continue
        out[num] = d[start:end]
    return out


def stream_of(body):
    m = re.search(rb'stream\r?\n', body)
    if not m:
        return None
    raw = body[m.end():]
    raw = raw.rstrip()
    if raw.endswith(b'endstream'):
        raw = raw[:-len(b'endstream')].rstrip(b'\r\n')
    if b'/FlateDecode' in body[:m.start()]:
        try:
            return zlib.decompress(raw)
        except zlib.error:
            return zlib.decompressobj().decompress(raw)
    return raw


def pages(d, objs):
    """Page object numbers in document order, from the /Kids tree."""
    root = None
    for num, body in objs.items():
        if b'/Type' in body and b'/Pages' in body and b'/Kids' in body and b'/Parent' not in body:
            root = num
    order = []

    def walk(n, seen):
        if n in seen:
            return
        seen.add(n)
        body = objs.get(n, b'')
        if b'/Kids' in body:
            kids = re.search(rb'/Kids\s*\[(.*?)\]', body, re.S)
            for k in re.finditer(rb'(\d+)\s+\d+\s+R', kids.group(1)):
                walk(int(k.group(1)), seen)
        elif b'/Page' in body:
            order.append(n)

    if root is not None:
        walk(root, set())
    if not order:
        order = sorted(n for n, b in objs.items()
                       if re.search(rb'/Type\s*/Page\b', b))
    return order


def content(d, objs, pnum):
    body = objs[pnum]
    m = re.search(rb'/Contents\s+(\d+)\s+\d+\s+R', body)
    if m:
        return stream_of(objs[int(m.group(1))]) or b''
    m = re.search(rb'/Contents\s*\[(.*?)\]', body, re.S)
    out = b''
    if m:
        for k in re.finditer(rb'(\d+)\s+\d+\s+R', m.group(1)):
            out += (stream_of(objs[int(k.group(1))]) or b'') + b'\n'
    return out


TOK = re.compile(rb'/[^\s/\[\]<>()]+|-?\d*\.?\d+|\[|\]|\(|<<|>>|[A-Za-z\'"*]+')


def dump(path, page_index, want=None):
    d = open(path, 'rb').read()
    objs = objects(d)
    order = pages(d, objs)
    cs = content(d, objs, order[page_index - 1])
    # font name -> base font, from the page's /Resources
    body = objs[order[page_index - 1]]
    res = {}
    m = re.search(rb'/Font\s*<<(.*?)>>', body, re.S)
    src = m.group(1) if m else b''
    if not m:
        rm = re.search(rb'/Resources\s+(\d+)\s+\d+\s+R', body)
        if rm:
            rb_ = objs[int(rm.group(1))]
            m2 = re.search(rb'/Font\s*<<(.*?)>>', rb_, re.S)
            if m2:
                src = m2.group(1)
    for fm in re.finditer(rb'/(\w+)\s+(\d+)\s+\d+\s+R', src):
        fo = objs.get(int(fm.group(2)), b'')
        bf = re.search(rb'/BaseFont\s*/([^\s/\]>]+)', fo)
        res[fm.group(1).decode()] = bf.group(1).decode() if bf else '?'

    toks = TOK.findall(cs)
    stack, font, size, tm, ln = [], '?', 0.0, None, None
    out = []
    i = 0
    while i < len(toks):
        t = toks[i]
        if t == b'Tf':
            size = float(stack[-1])
            font = stack[-2].decode().lstrip('/')
        elif t in (b'Tm',):
            tm = [float(x) for x in stack[-6:]]
        elif t in (b'Td', b'TD'):
            ln = [float(x) for x in stack[-2:]]
        elif t in (b'Tj', b'TJ', b"'", b'"'):
            out.append((font, res.get(font, '?'), size, tm, ln))
        elif t == b'TL':
            out.append(('TL', '', float(stack[-1]), None, None))
        if re.fullmatch(rb'-?\d*\.?\d+|/[^\s]+', t):
            stack.append(t)
        else:
            stack = stack[-8:]
        i += 1
    return out


if __name__ == '__main__':
    path, page = sys.argv[1], int(sys.argv[2])
    seen = set()
    for font, base, size, tm, ln in dump(path, page):
        key = (font, base, size, tuple(tm or ()), tuple(ln or ()))
        if key in seen:
            continue
        seen.add(key)
        print(f'{base:34s} size={size:8.4f} Tm={tm} Td={ln}')
