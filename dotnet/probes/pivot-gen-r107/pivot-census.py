#!/usr/bin/env python3
"""Census every pivot table of a workbook, and say whether the grid is generated for it.

    pivot-census.py <workbook> ...

One line per pivot part: the sheet it draws on, the geometry it states, whether its cache reads a
range of this workbook, and the accept/decline the predictor reaches — which is
`pivot-grid.py`'s own, so the census and the prediction cannot drift apart.

    rowDataPH   the row axis carries the data-layout placeholder (`<field x="-2"/>`)
    drill       `showDrill`, which is the whole of the row-header indent
"""
import sys, os, zipfile, importlib.util
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('pg', os.path.join(HERE, 'pivot-grid.py'))
pg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pg)
m = pg.m


def cache_type(z, part):
    rp = os.path.join(os.path.dirname(part), '_rels', os.path.basename(part) + '.rels')
    if rp not in z.namelist():
        return 'none'
    for r in ET.fromstring(z.read(rp)):
        if not r.get('Type', '').endswith('/pivotCacheDefinition'):
            continue
        if r.get('TargetMode') == 'External':
            return 'external-part'
        raw = r.get('Target')
        t = (raw.lstrip('/') if raw.startswith('/')
             else os.path.normpath(os.path.join(os.path.dirname(part), raw))).replace('\\', '/')
        if t not in z.namelist():
            continue
        src = ET.fromstring(z.read(t)).find(m('cacheSource'))
        return (src.get('type') if src is not None else '?') or '?'
    return 'none'


def main():
    for path in sys.argv[1:]:
        z = zipfile.ZipFile(path)
        name = os.path.basename(path)
        seen = False
        for sheet, pivots in pg.pivots_by_sheet(z):
            for part, root, cached in pivots:
                seen = True
                loc = root.find(m('location'))
                rf = [int(f.get('x', '-1')) for f in pg.children(root.find(m('rowFields')), 'field')]
                cf = [int(f.get('x', '-1')) for f in pg.children(root.find(m('colFields')), 'field')]
                nd = len(pg.children(root.find(m('dataFields')), 'dataField'))
                npg = len(pg.children(root.find(m('pageFields')), 'pageField'))
                nri = len(pg.children(root.find(m('rowItems')), 'i'))
                nci = len(pg.children(root.find(m('colItems')), 'i'))
                o, why = pg.generate(root, cached)
                print('%s\t%s\t%s\tref=%s fhr=%s fdr=%s fdc=%s rf=%s cf=%s df=%d pf=%d ri=%d ci=%d '
                      'drill=%s cache=%s rowDataPH=%s\t%s\t%s'
                      % (name, sheet, os.path.basename(part), loc.get('ref'),
                         loc.get('firstHeaderRow', '1'), loc.get('firstDataRow', '1'),
                         loc.get('firstDataCol', '0'), rf, cf, nd, npg, nri, nci,
                         pg.flag(root, 'showDrill', True), cache_type(z, part),
                         any(x < 0 for x in rf),
                         'ACCEPT' if o is not None else 'decline', why or ''))
        if not seen:
            print('%s\t-\t-\tno pivot parts\t-\t-' % name)


main()
