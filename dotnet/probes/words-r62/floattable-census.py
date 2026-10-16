#!/usr/bin/env python3
"""Which corpus documents hold a positioned table that runs off the bottom of its page?

A `w:tblpPr` table becomes a *fly* in Writer, and 26.2.4.2 always marks that fly splittable —
`DomainMapperTableHandler.cxx`:1765, "A text frame created for floating tables is always allowed
to split". `PlaceFloatedTable` places such a table whole or not at all, so the rows past the
bottom of the body are drawn off the page and the continuation page is never created.

The estimate here is deliberately crude and its limits are stated rather than hidden:

  * a row's height is taken from `w:trHeight` when it states one and from a floor otherwise, so
    every table whose rows size to their content is UNDER-counted;
  * `w:vertAnchor="page"` and `"margin"` are resolved against the page and the text area, `text`
    (the default) against the top of the text area, which is where a table that follows only a
    heading actually lands but NOT where one half way down the flow does — that arm is
    UNDER-counted too, because `used` is not known here;
  * only the body part is read: a table in a header, a footer or a text box is invisible to it;
  * only `.docx`. The 66 `.doc` files are not read at all, and neither is any ODF text document.

So the figure it prints is a *candidate* list, not a prediction of reach. Reach is measured by
rendering.
"""
import collections
import glob
import os
import re
import sys
import zipfile

ROOT = '/c/sandbox/workdir/sample-files'
NS = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'

import xml.etree.ElementTree as ET


def w(tag):
    return NS + tag


def attr(el, name):
    return None if el is None else el.get(w(name))


def sect_geometry(body):
    """The last sectPr, as (body width, body height) in points."""
    sects = body.findall('.//' + w('sectPr'))
    if not sects:
        return None
    s = sects[-1]
    sz = s.find(w('pgSz'))
    mar = s.find(w('pgMar'))
    if sz is None or mar is None:
        return None
    try:
        h = int(sz.get(w('h')))
        top = int(mar.get(w('top')))
        bot = int(mar.get(w('bottom')))
    except (TypeError, ValueError):
        return None
    header = int(mar.get(w('header')) or 0)
    return (h - top - bot) / 20.0, top / 20.0, header / 20.0


ROW_FLOOR_TWIPS = 240  # one 12 pt line and a little cell padding; deliberately small.


def rows_of(tbl):
    return [r for r in tbl if r.tag == w('tr')]


def row_height(tr):
    pr = tr.find(w('trPr'))
    if pr is not None:
        th = pr.find(w('trHeight'))
        if th is not None and th.get(w('val')):
            try:
                return int(th.get(w('val')))
            except ValueError:
                pass
    return ROW_FLOOR_TWIPS


def scan(path):
    out = []
    try:
        with zipfile.ZipFile(path) as z:
            xml = z.read('word/document.xml')
    except Exception as exc:                                   # noqa: BLE001
        return [('UNREADABLE', str(exc))]
    root = ET.fromstring(xml)
    body = root.find(w('body'))
    if body is None:
        return out
    geom = sect_geometry(body)
    if geom is None:
        return out
    body_h, top_margin, _header = geom

    for tbl in body.iter(w('tbl')):
        pr = tbl.find(w('tblPr'))
        if pr is None:
            continue
        pos = pr.find(w('tblpPr'))
        if pos is None:
            continue
        try:
            y = int(pos.get(w('tblpY')) or 0) / 20.0
        except ValueError:
            y = 0.0
        anchor = pos.get(w('vertAnchor')) or 'text'
        # Where the table's top lands relative to the top of the text area.
        if anchor == 'page':
            top = y - top_margin
        elif anchor == 'margin':
            top = y
        else:
            top = y                                   # `text`, with `used` assumed nought.
        rows = rows_of(tbl)
        height = sum(row_height(r) for r in rows) / 20.0
        out.append((len(rows), round(top, 2), round(height, 2), round(body_h, 2),
                    round(top + height - body_h, 2), anchor))
    return out


def main():
    paths = []
    with open(os.path.join(ROOT, 'MANIFEST.tsv'), encoding='utf-8') as f:
        next(f)
        for line in f:
            p = line.rstrip('\n').split('\t')
            if p[0] == 'words' and p[3] == 'docx':
                paths.append(p[2])

    docs_with_positioned = 0
    tables = 0
    overflowing_docs = []
    overflow_tables = 0
    anchors = collections.Counter()

    for rel in paths:
        found = scan(os.path.join(ROOT, rel))
        if not found:
            continue
        if found and found[0][0] == 'UNREADABLE':
            print('UNREADABLE', rel, found[0][1], file=sys.stderr)
            continue
        docs_with_positioned += 1
        tables += len(found)
        worst = None
        for rows, top, height, body_h, over, anchor in found:
            anchors[anchor] += 1
            if over > 0.0:
                overflow_tables += 1
                if worst is None or over > worst[4]:
                    worst = (rows, top, height, body_h, over, anchor)
        if worst:
            overflowing_docs.append((rel, worst))

    print('documents holding a positioned table : %4d of %d .docx' % (docs_with_positioned, len(paths)))
    print('positioned tables                    : %4d' % tables)
    print('vertAnchor                           : %s' % dict(anchors))
    print('tables whose declared rows overflow  : %4d in %d documents' % (
        overflow_tables, len(overflowing_docs)))
    print()
    for rel, (rows, top, height, body_h, over, anchor) in sorted(
            overflowing_docs, key=lambda r: -r[1][4]):
        print('  %8.2f pt over   rows %3d  top %7.2f  height %8.2f  body %7.2f  anchor %-6s  %s'
              % (over, rows, top, height, body_h, anchor, rel))


if __name__ == '__main__':
    main()
