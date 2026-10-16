#!/usr/bin/env python3
r"""`rtf-style-r97/readfodt.py` with the columns the heading question turns on.

Same instrument, same two-part resolution -- direct character formatting on the paragraph's first
span beats the whole `style:parent-style-name` chain, because that is where the reference writes
the reset it lays over a style's own `\fs`, and a reader that walks only the style chain reports
the opposite answer on that arm (r97 §2, and reproduced here 20 of 20 before this was trusted on
headings).

What is added is the four properties `COLL_HEADLINE_BASE` does *not* state and one it does, so
that "the walk continues into Standard" can be told from "the intermediate shadows everything":

    colour   fo:color            not stated by the intermediate
    align    fo:text-align       not stated
    left     fo:margin-left      not stated
    font     style:font-name     **stated** -- three scripts of it, `:795-807`
    size     fo:font-size        **stated** -- `PT_14`, `:809`

A value's `@Name` suffix is the chain level it was found at, so `bold@Standard` is the document's
own `Normal` reaching the paragraph and a bare `bold` is the paragraph's own automatic style.
A `@direct` is the run's own character formatting.

  readfodt2.py <dir of .fodt> [<second dir>]
"""
import pathlib
import sys
import xml.etree.ElementTree as ET

NS = {
    'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
    'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
    'text': 'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
    'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
}
Q = {k: '{%s}' % v for k, v in NS.items()}

WANT = [
    ('size', Q['fo'] + 'font-size', 'text'),
    ('weight', Q['fo'] + 'font-weight', 'text'),
    ('italic', Q['fo'] + 'font-style', 'text'),
    ('colour', Q['fo'] + 'color', 'text'),
    ('font', Q['style'] + 'font-name', 'text'),
    ('under', Q['style'] + 'text-underline-style', 'text'),
    ('strike', Q['style'] + 'text-line-through-style', 'text'),
    ('caps', Q['fo'] + 'text-transform', 'text'),
    ('align', Q['fo'] + 'text-align', 'para'),
    ('left', Q['fo'] + 'margin-left', 'para'),
    ('above', Q['fo'] + 'margin-top', 'para'),
    ('below', Q['fo'] + 'margin-bottom', 'para'),
    ('keep', Q['fo'] + 'keep-with-next', 'para'),
]
COLUMNS = [k for k, _, _ in WANT]


def family(root, want):
    out = {}
    for holder in ('styles', 'automatic-styles'):
        node = root.find(Q['office'] + holder)
        if node is None:
            continue
        for st in node.findall(Q['style'] + 'style'):
            if st.get(Q['style'] + 'family') != want:
                continue
            name = st.get(Q['style'] + 'name')
            if want == 'text':
                out[name] = st.find(Q['style'] + 'text-properties')
            else:
                out[name] = {
                    'parent': st.get(Q['style'] + 'parent-style-name'),
                    'para': st.find(Q['style'] + 'paragraph-properties'),
                    'text': st.find(Q['style'] + 'text-properties'),
                }
    return out


def resolve(styles, start, direct):
    chain, seen, cur = [], set(), start
    while cur and cur in styles and cur not in seen:
        seen.add(cur)
        chain.append(cur)
        cur = styles[cur]['parent']
    row = {'chain': '>'.join(chain)}
    for key, attr, where in WANT:
        if where == 'text' and direct is not None and direct.get(attr) is not None:
            row[key] = direct.get(attr) + '@direct'
            continue
        for depth, name in enumerate(chain):
            node = styles[name][where]
            if node is not None and node.get(attr) is not None:
                row[key] = node.get(attr) + ('' if depth == 0 else '@' + chain[depth])
                break
        else:
            row[key] = '-'
    return row


def read(path):
    root = ET.parse(path).getroot()
    styles = family(root, 'paragraph')
    texts = family(root, 'text')
    body = root.find(Q['office'] + 'body/' + Q['office'] + 'text')
    for p in body.iter(Q['text'] + 'p'):
        if 'HEAD' not in ''.join(p.itertext()):
            continue
        span = next((s for s in p.iter(Q['text'] + 'span') if ''.join(s.itertext())), None)
        direct = texts.get(span.get(Q['text'] + 'style-name')) if span is not None else None
        return resolve(styles, p.get(Q['text'] + 'style-name'), direct)
    return None


dirs = [pathlib.Path(d) for d in sys.argv[1:]] or [pathlib.Path('.')]
names = sorted({p.stem for d in dirs for p in d.glob('p_*.fodt')})
width = max((len(n) for n in names), default=10)
head = f'{"probe":<{width}}  {"style chain":<38}'
for key in COLUMNS:
    head += f' {key:<15}'
print(head)
for name in names:
    for d in dirs:
        path = d / f'{name}.fodt'
        row = read(path) if path.exists() else None
        if row is None:
            print(f'{name:<{width}}  -')
            continue
        line = f'{name:<{width}}  {row["chain"]:<38}'
        for key in COLUMNS:
            line += f' {row[key]:<15}'
        print(line)
