#!/usr/bin/env python3
r"""Census `\charscalex` over the converted `.rtf` column, with its base rate beside it.

Three rules this follows, each of which a previous round paid for:

  * **count the non-identity occurrences.**  A run stating 100 costs nothing, and counting it
    overstates the reach -- the ODF census was 350 gross and 111 net.
  * **print a base rate.**  Round 145's `\charscalex` census returned 0 occurrences AND 0 base-rate
    tokens over an absent directory, which reads exactly like a nil-reach finding and is not one.
    A corpus of rich-text files cannot hold no font switches, so a zero base rate condemns the
    census rather than the corpus.
  * **separate the stylesheet from the body.**  `{\stylesheet ...}` declares; a paragraph applies.
    A value declared in a style and never used reaches no glyph, which is the same correction
    round 88 had to make to round 87's reach figure.

Usage: census-rtf.py [<dir>]
"""
import collections
import pathlib
import re
import sys

SCALE = re.compile(rb'\\charscalex(-?[0-9]+)')
FONT = re.compile(rb'\\f[0-9]+')
STYLESHEET = re.compile(rb'\{\\stylesheet')


def stylesheet_span(data):
    """The byte range of the `{\\stylesheet ...}` group, or None."""
    m = STYLESHEET.search(data)
    if not m:
        return None
    depth, at = 0, m.start()
    while at < len(data):
        c = data[at]
        if c == 0x5c:          # a backslash escapes the next byte
            at += 2
            continue
        if c == 0x7b:
            depth += 1
        elif c == 0x7d:
            depth -= 1
            if depth == 0:
                return (m.start(), at + 1)
        at += 1
    return (m.start(), len(data))


def main():
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/corpus-odf/rtf')
    files = sorted(root.glob('*.rtf'))
    values = collections.Counter()
    body_values = collections.Counter()
    docs_any, docs_net, docs_body_net = set(), set(), set()
    gross = net = body_gross = body_net = 0
    fonts = 0
    rows = []
    for path in files:
        data = path.read_bytes()
        fonts += len(FONT.findall(data))
        span = stylesheet_span(data)
        g = n = bg = bn = 0
        for m in SCALE.finditer(data):
            v = int(m.group(1))
            values[v] += 1
            g += 1
            in_style = span is not None and span[0] <= m.start() < span[1]
            if not in_style:
                body_values[v] += 1
                bg += 1
            if v != 100:
                n += 1
                if not in_style:
                    bn += 1
        gross += g
        net += n
        body_gross += bg
        body_net += bn
        if g:
            docs_any.add(path.name)
        if n:
            docs_net.add(path.name)
        if bn:
            docs_body_net.add(path.name)
        if g:
            rows.append((path.name, g, n, bg, bn))

    print('files                         : %d' % len(files))
    print('base rate, \\fN run tokens     : %d' % fonts)
    print('occurrences, gross            : %d in %d documents' % (gross, len(docs_any)))
    print('occurrences, NON-IDENTITY     : %d in %d documents' % (net, len(docs_net)))
    print('outside {\\stylesheet}, gross  : %d' % body_gross)
    print('outside {\\stylesheet}, net    : %d in %d documents' % (body_net, len(docs_body_net)))
    print('values (all)                  : %s' % dict(sorted(values.items())))
    print('values (outside stylesheet)   : %s' % dict(sorted(body_values.items())))
    print()
    print('document\tgross\tnet\tbody_gross\tbody_net')
    for row in sorted(rows, key=lambda r: -r[2]):
        print('%s\t%d\t%d\t%d\t%d' % row)


if __name__ == '__main__':
    main()
