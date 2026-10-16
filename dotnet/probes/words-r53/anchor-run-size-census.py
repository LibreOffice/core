#!/usr/bin/env python3
"""Runs whose whole content is a frame anchor and which state a size of their own.

    python3 anchor-run-size-census.py <corpus-root> [family ...]

A `w:drawing`, `w:pict`, `w:object` or `w:commentReference` reaches layout as U+0001 — a position
that stands for something which is not text.  The run holding it still carries a font size, and we
let that size decide the line's height.  **26.2.4.2 does not**, measured on ten authored variants
of `097_Business_Case_Template_Elegant_Layout` (`probes/words-r53/results.md`): a run holding only
a drawing adds the same height at 10 pt and at 26 pt, floating or as-character, with text beside it
or alone.

This counts the runs that can differ: a run whose element children are exactly one anchor-bearing
element and no `w:t`, `w:tab`, `w:br` or `w:sym`, **and** which states a `w:sz` in its own `w:rPr`.

**What it cannot see:**

* whether that stated size actually differs from the paragraph's resolved size — the census does
  not resolve styles, so it is an **upper bound**, and most inline pictures are set in the size of
  the text around them and will not move;
* a size inherited from a *character style* named by the run rather than stated on it, which is the
  mirror error and makes the count a floor as well;
* `.doc`, `.rtf` and `.odt`, whose readers use the same U+0001 convention;
* whether the run's line holds anything else, which is what decides if the difference is visible.
"""
import csv, os, sys, zipfile
from xml.etree import ElementTree as ET

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
ANCHORS = {'drawing', 'pict', 'object', 'commentReference'}
TEXTY = {'t', 'tab', 'br', 'sym', 'noBreakHyphen', 'ptab', 'softHyphen', 'instrText',
         'footnoteReference', 'endnoteReference', 'fldChar', 'delText'}


def count(path):
    runs = sized = 0
    try:
        with zipfile.ZipFile(path) as zf:
            for name in zf.namelist():
                if not name.startswith('word/') or not name.endswith('.xml'):
                    continue
                try:
                    root = ET.fromstring(zf.read(name))
                except ET.ParseError:
                    continue
                for run in root.iter(W + 'r'):
                    kinds = {c.tag[len(W):] for c in run if c.tag.startswith(W)}
                    if not (kinds & ANCHORS) or (kinds & TEXTY):
                        continue
                    runs += 1
                    props = run.find(W + 'rPr')
                    if props is not None and props.find(W + 'sz') is not None:
                        sized += 1
    except (zipfile.BadZipFile, OSError):
        return None
    return runs, sized


def main():
    root = sys.argv[1]
    families = sys.argv[2:] or ['words']
    rows = []
    with open(os.path.join(root, 'MANIFEST.tsv')) as f:
        for row in csv.DictReader(f, delimiter='\t'):
            if row['family'] in families:
                rows.append(row)

    total_runs = total_sized = docs = 0
    detail = []
    for row in rows:
        got = count(os.path.join(root, row['path']))
        if not got or not got[1]:
            continue
        total_runs += got[0]
        total_sized += got[1]
        docs += 1
        detail.append((got[1], got[0], row['status'], row['path']))

    print(f'{total_sized} anchor-only runs stating a size, of {total_runs} anchor-only runs, '
          f'in {docs} documents')
    for sized, runs, status, path in sorted(detail, reverse=True)[:25]:
        print(f'    {sized:4d} of {runs:4d}  {status:5s}  {path}')


if __name__ == '__main__':
    main()
