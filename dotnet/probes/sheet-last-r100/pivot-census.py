#!/usr/bin/env python3
"""Which sheets-corpus documents state a pivot table, and — for the OOXML ones — how much
`Pivot Table *` formatting 26.2.4.2 generates for it.

Two passes.  The first walks the zip: an `xl/pivotTables/pivotTableN.xml` part for the
SpreadsheetML spellings, a `table:data-pilot-table` element for ODF.  The second reads a
`--convert-to fods` of each OOXML document (produced separately, into `pfods/`) and counts the
automatic cell styles whose parent is one of Calc's six DataPilot output styles.

The occurrence column counts `table:style-name="ceN"` in the markup and is NOT a cell count:
ODF's `number-columns-repeated` compresses a run of identical cells into one attribute, so a
1013-row pivot can show 56.  It bounds presence, not size.
"""
import glob, os, re, sys, zipfile

ROOTS = ['/home/user/sample-files/sheets', '/home/user/corpus-odf/sheets']
EXT = {'.xlsx', '.xlsm', '.ods'}
PIVOT_PARENT = 'Pivot_20_Table_20_'


def states_pivot(path):
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return None
    try:
        names = z.namelist()
        if path.lower().endswith('.ods'):
            if 'content.xml' not in names:
                return None
            return 'ods-pilot' if b'data-pilot-table' in z.read('content.xml') else None
        parts = [n for n in names if n.startswith('xl/pivotTables/pivotTable')]
        return '%d pivotTable' % len(parts) if parts else None
    finally:
        z.close()


def styled(fods):
    s = open(fods, encoding='utf-8', errors='replace').read()
    styles = {}
    for m in re.finditer(
            r'<style:style\b([^>]*?)(?:/>|>((?:(?!</style:style>).)*?)</style:style>)', s, re.S):
        attrs, body = m.group(1), m.group(2) or ''
        name = re.search(r'style:name="([^"]+)"', attrs)
        parent = re.search(r'style:parent-style-name="([^"]+)"', attrs)
        if name and parent and parent.group(1).startswith(PIVOT_PARENT):
            styles[name.group(1)] = 'fo:border' in body
    occurrences = bordered = 0
    for name, has_border in styles.items():
        n = len(re.findall('table:style-name="%s"' % re.escape(name), s))
        occurrences += n
        if has_border:
            bordered += n
    return len(styles), occurrences, bordered


def main():
    scanned = 0
    hits = []
    for root in ROOTS:
        for p in sorted(glob.glob(root + '/**/*', recursive=True)):
            if not os.path.isfile(p) or os.path.splitext(p)[1].lower() not in EXT:
                continue
            scanned += 1
            kind = states_pivot(p)
            if kind:
                hits.append((p, kind))
    print('scanned %d, stating a pivot table %d' % (scanned, len(hits)))
    for p, kind in hits:
        print('   %-12s %s' % (kind, p))

    if len(sys.argv) > 1:
        print('\n%-58s %6s %6s %8s' % ('fods', 'styles', 'occurs', 'bordered'))
        for f in sorted(glob.glob(sys.argv[1] + '/*.fods')):
            print('%-58s %6d %6d %8d' % ((os.path.basename(f)[:58],) + styled(f)))


if __name__ == '__main__':
    main()
