#!/usr/bin/env python3
"""The `#00007E` banner on `pres_ioc_phuket.ppt` page 26, in every render given.

Round 94 measured it 103.38 pt tall at the reference and 64.88 here and wrote "Not
characterised".  C16 says a size difference means nothing until you know which primitive
each side used, so the paint operator is printed beside the rectangle; C11 says the
reference is not reproducible run to run, so this takes any number of renders and prints
them all and you compare the rows.

    banner-geometry.py <pdf> [<pdf> ...]
"""
import importlib.util, sys, pathlib

# <repo>/dotnet/probes/<this dir>/<this file> -> <repo>/.claude/skills/…
SKILL = (pathlib.Path(__file__).resolve().parents[3]
         / '.claude/skills/render-comparison/scripts/pdf-ops.py')
spec = importlib.util.spec_from_file_location('pdfops', SKILL)
po = importlib.util.module_from_spec(spec)
spec.loader.exec_module(po)

print('render\tband\tx0\ty0\tx1\ty1\twidth\theight\tcolour\top')
for pdf in sys.argv[1:]:
    for r in po.read(pdf):
        if r['page'] != 26 or r['kind'] != 'fill': continue
        if r.get('colour') != '#00007E': continue
        band = "banner" if r["y0"] > 300 else "footer-strip"
        print('%s\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%s\t%s' % (
            pathlib.Path(pdf).parent.name, band, r['x0'], r['y0'], r['x1'], r['y1'],
            r['x1'] - r['x0'], r['y1'] - r['y0'], r['colour'], r['op']))
