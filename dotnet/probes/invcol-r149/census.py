#!/usr/bin/env python3
"""Reach of print centring, both spellings, over both corpus columns.

What the rule PAINTS is the position of a sheet's whole printed block, so the unit
is a document whose *rendered* sheet names a page layout that states the attribute --
not a document that merely contains the string somewhere. LibreOffice writes page
layouts for styles no table uses (`Default`, `Report` are in every converted .ods),
so the string count and the reach are different numbers and both are printed.

Base rates are printed beside every count, because a glob over an absent column
returns 0 occurrences AND 0 base-rate tokens and the second is the tell.
"""
import re, zipfile, pathlib, sys, collections

ODF = pathlib.Path('/home/user/corpus-odf')
SRC = pathlib.Path('/home/user/sample-files')

def zread(z, name):
    try: return z.read(name).decode('utf-8', 'replace')
    except KeyError: return ''

def odf_scan(path):
    """-> (states_anywhere, reached_by_a_table, per-value counter, n_page_layouts)"""
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return None
    styles, content = zread(z, 'styles.xml'), zread(z, 'content.xml')
    layouts = {}          # layout name -> centring value
    for m in re.finditer(r'<style:page-layout style:name="([^"]+)"(.*?)(?=<style:page-layout |\Z)',
                         styles, re.S):
        v = re.search(r'style:table-centering="([^"]+)"', m.group(2))
        layouts[m.group(1)] = v.group(1) if v else None
    masters = {}          # master-page name -> layout name
    for m in re.finditer(r'<style:master-page style:name="([^"]+)"[^>]*'
                         r'style:page-layout-name="([^"]+)"', styles):
        masters[m.group(1)] = m.group(2)
    tstyles = {}          # table style name -> master-page name
    for m in re.finditer(r'<style:style style:name="([^"]+)" style:family="table"[^>]*'
                         r'style:master-page-name="([^"]+)"', content):
        tstyles[m.group(1)] = m.group(2)
    reached = collections.Counter()
    for t in re.finditer(r'<table:table [^>]*>', content):
        st = re.search(r'table:style-name="([^"]+)"', t.group(0))
        if not st: continue
        v = layouts.get(masters.get(tstyles.get(st.group(1), ''), ''))
        if v: reached[v] += 1
    anywhere = collections.Counter(v for v in layouts.values() if v)
    return anywhere, reached, len(layouts)

def ooxml_scan(path):
    try: z = zipfile.ZipFile(path)
    except Exception: return None
    h = v = 0
    sheets = 0
    for n in z.namelist():
        if re.fullmatch(r'xl/worksheets/sheet\d+\.xml', n):
            sheets += 1
            x = zread(z, n)
            m = re.search(r'<printOptions[^>]*>', x)
            if m:
                if 'horizontalCentered="1"' in m.group(0) or 'horizontalCentered="true"' in m.group(0): h += 1
                if 'verticalCentered="1"' in m.group(0) or 'verticalCentered="true"' in m.group(0): v += 1
    return h, v, sheets

print('=== converted ODF corpus: style:table-centering ===')
for col in ('ods', 'odt', 'odp', 'rtf'):
    d = ODF / col
    files = sorted(d.glob(f'*.{col}')) if d.is_dir() else []
    if not d.is_dir():
        print(f'{col:4s} DIRECTORY ABSENT -- 0 occurrences and 0 base-rate documents')
        continue
    if col == 'rtf':
        print(f'{col:4s} {len(files)} documents, not a zip container -- not scanned '
              f'(base rate present, instrument does not apply)')
        continue
    docs_any = docs_reached = 0
    vals_any, vals_reached = collections.Counter(), collections.Counter()
    tables_reached = 0
    hits = []
    for f in files:
        r = odf_scan(f)
        if r is None: continue
        anywhere, reached, nlay = r
        if anywhere: docs_any += 1; vals_any += anywhere
        if reached:
            docs_reached += 1; vals_reached += reached
            tables_reached += sum(reached.values())
            hits.append((f.name, dict(reached)))
    print(f'{col:4s} base rate {len(files)} documents')
    print(f'     states it in any page layout : {docs_any:4d}  values {dict(vals_any)}')
    print(f'     a TABLE reaches it           : {docs_reached:4d}  '
          f'({tables_reached} tables)  values {dict(vals_reached)}')
    if col == 'ods':
        pathlib.Path('census-ods-hits.tsv').write_text(
            'document\treached_by_tables\n' +
            ''.join(f'{n}\t{v}\n' for n, v in hits))

print()
print('=== original corpus: printOptions/@horizontalCentered (the OOXML spelling we DO read) ===')
man = [l.split('\t') for l in (SRC / 'MANIFEST.tsv').read_text().splitlines()[1:]]
xl = [r for r in man if r[3] in ('xlsx', 'xlsm')]
print(f'base rate {len(xl)} xlsx/xlsm rows in MANIFEST.tsv '
      f'(of {len(man)} documents, {sum(1 for r in man if r[0]=="sheets")} sheets)')
dh = dv = 0; sh = sv = 0; tot = 0
for r in xl:
    p = SRC / r[2]
    if not p.exists(): continue
    tot += 1
    o = ooxml_scan(p)
    if not o: continue
    h, v, n = o
    if h: dh += 1; sh += h
    if v: dv += 1; sv += v
print(f'scanned {tot}; horizontalCentered on {dh} documents ({sh} sheets), '
      f'verticalCentered on {dv} documents ({sv} sheets)')
