#!/usr/bin/env python3
"""Shapes in the words corpus whose fill states a transparency, by reader and by darkness.

Sizes both halves of the automatic-colour change:

  * how many VML shapes state a `v:fill/@opacity`, which `DocxVmlFrames` reads for nothing today —
    so reading it changes the *drawn* fill as well as the text colour;
  * how many DrawingML shape fills state an `a:alpha`;
  * and, for each, whether the fill is dark *before* the blend and bright *after* it — because a
    fill that is dark either way, or bright either way, cannot tell the two readings apart. That
    count is the population where round 59's counter-witnesses live.

Blind spots stated before the sweep: a themed or scheme colour is not resolved here (only literal
`#rrggbb` / `a:srgbClr`), a gradient fill is skipped entirely, and whether the shape holds any text
at all — let alone text stating no colour — is not read. So every figure here is an upper bound on
the shapes whose text colour can move, and a lower bound on the palette, since themed fills are
missed.

    shapealpha-census.py
"""
import collections
import os
import re
import sys
import zipfile

MANIFEST = '/c/sandbox/workdir/sample-files/MANIFEST.tsv'
ROOT = '/c/sandbox/workdir/sample-files'
VML = re.compile(rb'<v:(rect|roundrect|shape)\b[^>]*?(?:fillcolor="#([0-9A-Fa-f]{6})[^"]*")[^>]*>'
                 rb'(?:\s*<v:fill\b[^>]*opacity="([^"]+)"[^>]*/?>)?')
DML = re.compile(rb'<a:solidFill>\s*<a:srgbClr val="([0-9A-Fa-f]{6})"\s*(?:/>|>'
                 rb'\s*<a:alpha val="(\d+)"\s*/>)')


def decoded(c):
    v = c / 255.0
    return v / 12.92 if v < 0.04045 else ((v + 0.055) / 1.055) ** 2.4


def wcag(rgb):
    return int((decoded(rgb[0]) * 0.2126 + decoded(rgb[1]) * 0.7152 + decoded(rgb[2]) * 0.0722) * 255)


def blend(rgb, transparency):
    return tuple(round(c + (255 - c) * transparency) for c in rgb)


def opacity_of(raw):
    """A VML opacity: `26214f` is 16.16 fixed point, `40%` a percentage, `.4` a fraction."""
    text = raw.decode('ascii', 'replace').strip()
    if text.endswith('f'):
        return float(text[:-1]) / 65536.0
    if text.endswith('%'):
        return float(text[:-1]) / 100.0
    value = float(text)
    return value / 65536.0 if value > 1.0 else value


def main():
    rows = [l.rstrip('\n').split('\t') for l in open(MANIFEST, encoding='utf-8').readlines()[1:]]
    vml_docs = collections.Counter()
    dml_docs = collections.Counter()
    flips = []
    vml_total = dml_total = 0
    for r in rows:
        if len(r) < 4 or r[0] != 'words' or r[3] != 'docx':
            continue
        try:
            with zipfile.ZipFile(os.path.join(ROOT, r[2])) as z:
                blob = b''.join(z.read(n) for n in z.namelist()
                                if re.fullmatch(r'word/(document|header\d*|footer\d*)\.xml', n))
        except Exception:                                          # noqa: BLE001
            continue
        for m in VML.finditer(blob):
            if not m.group(3):
                continue
            vml_total += 1
            vml_docs[r[2]] += 1
            rgb = tuple(int(m.group(2)[i:i + 2], 16) for i in (0, 2, 4))
            t = 1.0 - opacity_of(m.group(3))
            if wcag(rgb) <= 87 < wcag(blend(rgb, t)):
                flips.append((r[2], 'vml', '#' + m.group(2).decode(), round(t * 100, 1)))
        for m in DML.finditer(blob):
            if not m.group(2):
                continue
            dml_total += 1
            dml_docs[r[2]] += 1
            rgb = tuple(int(m.group(1)[i:i + 2], 16) for i in (0, 2, 4))
            t = 1.0 - int(m.group(2)) / 100000.0
            if wcag(rgb) <= 87 < wcag(blend(rgb, t)):
                flips.append((r[2], 'dml', '#' + m.group(1).decode(), round(t * 100, 1)))

    print('VML shapes stating a v:fill/@opacity : %d in %d documents' % (vml_total, len(vml_docs)))
    for path, n in vml_docs.most_common():
        print('    %-4d %s' % (n, path))
    print('DrawingML solid fills stating an a:alpha : %d in %d documents'
          % (dml_total, len(dml_docs)))
    for path, n in dml_docs.most_common():
        print('    %-4d %s' % (n, path))
    print()
    print('fills that are DARK opaque and BRIGHT once blended — where the two readings differ: %d'
          % len(flips))
    for path, kind, colour, t in flips:
        print('    %-4s %-9s %4.1f%% transparent   %s' % (kind, colour, t, path))


if __name__ == '__main__':
    main()
