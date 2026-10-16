#!/usr/bin/env python3
"""One-attribute variants of 053_Personal_asset_inventory, to find what decides whether
26.2.4.2 draws the chart's seventh category.

The chart names `Assets!$H$24:$H$30` -- seven cells of a pivot table's output -- and caches
six points; the seventh, `Grand Total`, is in the sheet and not in the cache.  The workbook
also holds an Excel table whose `displayName` is `Assets`, which is the name of the sheet the
chart references.

    pivot-variants.py <outdir>
"""
import pathlib
import shutil
import sys
import zipfile

SRC = pathlib.Path('/home/user/sample-files/sheets/chartset-009/xlsx/'
                   '053_Personal_asset_inventory_5446d84b.xlsx')


def write(out, name, edits):
    """Copy the package, applying `edits` (part -> function of bytes)."""
    dst = out / name
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            fn = edits.get(item.filename)
            if fn is not None:
                data = fn(data)
            zout.writestr(item, data)
    return dst


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)

    # 0. The control: repackaged and otherwise untouched.
    write(out, 'v0-control.xlsx', {})

    # 1. The table renamed, so nothing in the workbook is called `Assets` but the sheet.
    ren = lambda b: (b.replace(b'name="Assets" displayName="Assets"',
                               b'name="Tbl9" displayName="Tbl9"'))
    renref = lambda b: b.replace(b'Assets[', b'Tbl9[')
    write(out, 'v1-table-renamed.xlsx', {
        'xl/tables/table11.xml': ren,
        'xl/workbook.xml': renref,
    })

    # 2. The table part dropped entirely (and its relationship and tableParts with it).
    def drop_parts(b):
        return b.replace(b'<tableParts count="1"><tablePart r:id="rId4"/></tableParts>', b'')

    def drop_rel(b):
        import re
        return re.sub(rb'<Relationship[^>]*table11\.xml"[^>]*/>', b'', b)

    def drop_ct(b):
        import re
        return re.sub(rb'<Override PartName="/xl/tables/table11\.xml"[^>]*/>', b'', b)

    dst = out / 'v2-no-table.xlsx'
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            if item.filename == 'xl/tables/table11.xml':
                continue
            data = zin.read(item.filename)
            if item.filename == 'xl/worksheets/sheet11.xml':
                data = drop_parts(data)
            elif item.filename == 'xl/worksheets/_rels/sheet11.xml.rels':
                data = drop_rel(data)
            elif item.filename == '[Content_Types].xml':
                data = drop_ct(data)
            zout.writestr(item, data)

    # 3. The chart's own cache given the seventh point, everything else untouched.
    #    The control from `probes/numfmt-r68`: it establishes that the scale follows the
    #    data, so a difference in what is drawn is a difference in the data.
    def add_point(b):
        s = b.decode('utf-8')
        s = s.replace('<c:ptCount val="6"/><c:pt idx="0"><c:v>Car</c:v></c:pt>',
                      '<c:ptCount val="7"/><c:pt idx="0"><c:v>Car</c:v></c:pt>', 1)
        s = s.replace('<c:pt idx="5"><c:v>House</c:v></c:pt>',
                      '<c:pt idx="5"><c:v>House</c:v></c:pt>'
                      '<c:pt idx="6"><c:v>Grand Total</c:v></c:pt>', 1)
        s = s.replace('<c:ptCount val="6"/><c:pt idx="0"><c:v>7500</c:v></c:pt>',
                      '<c:ptCount val="7"/><c:pt idx="0"><c:v>7500</c:v></c:pt>', 1)
        s = s.replace('<c:pt idx="5"><c:v>250000</c:v></c:pt>',
                      '<c:pt idx="5"><c:v>250000</c:v></c:pt>'
                      '<c:pt idx="6"><c:v>363500</c:v></c:pt>', 1)
        return s.encode('utf-8')

    write(out, 'v3-cache-has-seven.xlsx', {'xl/charts/chart11.xml': add_point})

    # 4. One value of the *sheet* changed and the chart's cache left alone, which is the
    #    question "does the reference read the sheet or the cache" asked directly.
    write(out, 'v4-sheet-value-changed.xlsx', {
        'xl/worksheets/sheet11.xml':
            lambda b: b.replace(b'<c r="I24" s="7"><v>7500</v></c>',
                                b'<c r="I24" s="7"><v>99000</v></c>'),
    })

    # 5. The pivot table part dropped, so nothing declares H23:I30 to be a DataPilot's
    #    output and the cells stay as the file wrote them.
    dst = out / 'v5-no-pivot.xlsx'
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            if item.filename.startswith('xl/pivotTables/'):
                continue
            data = zin.read(item.filename)
            if item.filename == 'xl/worksheets/_rels/sheet11.xml.rels':
                import re
                data = re.sub(rb'<Relationship[^>]*pivotTable1\.xml"[^>]*/>', b'', data)
            elif item.filename == '[Content_Types].xml':
                import re
                data = re.sub(rb'<Override PartName="/xl/pivotTables/pivotTable1\.xml"[^>]*/>',
                              b'', data)
            zout.writestr(item, data)

    # 6. The chart pointed at the table's own visible cells instead of the pivot's output,
    #    with its cache left alone.  If the reference then draws the table's order and
    #    values it is reading the sheet; if it keeps drawing the cache it never does.
    def retarget(b):
        s = b.decode('utf-8')
        s = s.replace('Assets!$H$24:$H$30', 'Assets!$B$5:$B$10')
        s = s.replace('Assets!$I$24:$I$30', 'Assets!$C$5:$C$10')
        return s.encode('utf-8')

    write(out, 'v6-chart-on-table.xlsx', {'xl/charts/chart11.xml': retarget})

    # 7. Columns H and I unhidden.  They carry the pivot's output, and the chart's range
    #    is inside them.
    write(out, 'v7-columns-shown.xlsx', {
        'xl/worksheets/sheet11.xml':
            lambda b: b.replace(b'width="14.5" style="2" hidden="1"', b'width="14.5" style="2"')
                       .replace(b'width="3.375" style="2" hidden="1"', b'width="3.375" style="2"'),
    })

    # 8 and 9. One of the two hidden columns shown, to tell the category range's own
    #    hiding from the value range's.
    write(out, 'v8-value-column-shown.xlsx', {
        'xl/worksheets/sheet11.xml':
            lambda b: b.replace(b'width="3.375" style="2" hidden="1"', b'width="3.375" style="2"'),
    })
    write(out, 'v9-category-column-shown.xlsx', {
        'xl/worksheets/sheet11.xml':
            lambda b: b.replace(b'width="14.5" style="2" hidden="1"', b'width="14.5" style="2"'),
    })

    # 10. The chart's cached points removed, its formulas left.  If the rendering does not
    #     move, the cache is not what is being drawn.
    def strip_cache(b):
        import re
        s = b.decode('utf-8')
        s = re.sub(r'<c:pt idx="\d+"><c:v>[^<]*</c:v></c:pt>', '', s)
        return s.encode('utf-8')

    write(out, 'v10-no-cache.xlsx', {'xl/charts/chart11.xml': strip_cache})

    # 11. v8 and v10 together: the value column shown, the category column still hidden,
    #     and the chart's cache stripped.  It asks which of the two supplies v8's six
    #     points -- the sheet's visible value column, or the cache.
    write(out, 'v11-value-column-shown-no-cache.xlsx', {
        'xl/worksheets/sheet11.xml':
            lambda b: b.replace(b'width="3.375" style="2" hidden="1"', b'width="3.375" style="2"'),
        'xl/charts/chart11.xml': strip_cache,
    })

    print('\n'.join(sorted(p.name for p in out.iterdir())))


if __name__ == '__main__':
    main()
