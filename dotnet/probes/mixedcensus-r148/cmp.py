#!/usr/bin/env python3
"""Run both censuses' mixed() on one PDF, per page, and dump the grouping."""
import sys, os, importlib.util, collections
import pymupdf

R146 = '/home/user/libreoffice-core/dotnet/probes/tablerow-r146'
sys.path.insert(0, R146)

# import census-pdf.py's helpers without running its main body
import types
src = open(os.path.join(R146, 'census-pdf.py')).read()
cut = src.index('rows = []')
mod = types.ModuleType('cpdf')
mod.__dict__['__name__'] = 'cpdf'
exec(compile(src[:cut], 'census-pdf.py', 'exec'), mod.__dict__)

spec = importlib.util.spec_from_file_location(
    'mixedlines', '/home/user/libreoffice-core/dotnet/probes/tbalign-r147/mixed-lines.py')
ml = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ml)


def ml_page(page):
    lines = collections.defaultdict(set)
    for item in page.get_drawings():
        kind = item.get('type', '')
        for segment in item['items']:
            if segment[0] == 'l':
                a, b = segment[1], segment[2]
                if abs(a.y - b.y) > 0.01 or abs(a.x - b.x) < 4:
                    continue
                width = round(item.get('width') or 0.0, 3)
                lines[round(a.y - (width / 2), 2)].add(width)
            elif segment[0] == 're' and kind in ('f', 'fs', 's'):
                box = segment[1]
                if box.height > 6.0 or box.width < 4.0:
                    continue
                lines[round(box.y0, 2)].add(round(box.height, 3))
    return sum(1 for w in lines.values() if len(w) > 1), len(lines), lines


path = sys.argv[1]
pages = [int(x) for x in sys.argv[2:]] or None
doc = pymupdf.open(path)
tot_a = tot_b = 0
for p in range(doc.page_count):
    if pages is not None and p not in pages:
        continue
    page = doc[p]
    H, V = mod.segs(page)
    a, same = mod.mixed(H)
    b, nl, lines = ml_page(page)
    tot_a += a; tot_b += b
    print(f'page {p}: census-pdf mixed={a} same={same} | mixed-lines mixed={b} of {nl} groups | H segs={len(H)}')
print(f'TOTAL census-pdf={tot_a}  mixed-lines={tot_b}')
