#!/usr/bin/env python3
"""Compare our XLSX column-width arithmetic against LibreOffice's own flat-ODF answer."""
import zipfile, re, sys, math

XLSX, FODS = sys.argv[1], sys.argv[2]
DIGIT = float(sys.argv[3])          # twips per digit as our code would compute it

# ---- LibreOffice's answer: per table, the resolved column widths in twips ----
s = open(FODS, encoding='utf-8').read()
styles = {}
for m in re.finditer(r'<style:style style:name="(co\d+)"[^>]*>\s*<style:table-column-properties([^>]*)/>', s):
    w = re.search(r'style:column-width="([\d.]+)(in|cm|mm|pt)"', m.group(2))
    v, u = float(w.group(1)), w.group(2)
    tw = v * {'in': 1440, 'cm': 1440/2.54, 'mm': 144/2.54, 'pt': 20}[u]
    styles[m.group(1)] = tw

tables = {}
for tm in re.finditer(r'<table:table table:name="([^"]*)"[^>]*>(.*?)<table:table-row', s, re.S):
    name, head = tm.group(1), tm.group(2)
    cols = []
    for cm in re.finditer(r'<table:table-column[^>]*?/>', head):
        tag = cm.group(0)
        st = re.search(r'table:style-name="(co\d+)"', tag)
        rep = re.search(r'table:number-columns-repeated="(\d+)"', tag)
        n = int(rep.group(1)) if rep else 1
        vis = 'table:visibility="collapse"' in tag or 'table:visibility="filter"' in tag
        if st:
            cols.extend([(styles[st.group(1)], vis)] * n)
    tables[name] = cols

# ---- our answer ----
z = zipfile.ZipFile(XLSX)
wb = z.read('xl/workbook.xml').decode('utf8')
rels = z.read('xl/_rels/workbook.xml.rels').decode('utf8')
relmap = {m.group(1): m.group(2) for m in
          re.finditer(r'<Relationship Id="([^"]+)"[^>]*Target="([^"]+)"', rels)}
sheets = [(m.group(2), relmap[m.group(1)]) for m in
          re.finditer(r'<sheet name="([^"]*)"[^>]*r:id="([^"]*)"', wb)
          for m2 in [None]] if False else []
sheets = []
for m in re.finditer(r'<sheet ([^>]*)/>', wb):
    a = m.group(1)
    nm = re.search(r'name="([^"]*)"', a).group(1)
    rid = re.search(r'r:id="([^"]*)"', a).group(1)
    sheets.append((nm, relmap[rid]))

BASE_PAD = 75.0        # five 96-dpi pixels
BIAS = 0.5

def at(digits, bias):
    t = digits * DIGIT + bias
    return 0 if t <= 0 else int(t)

for name, target in sheets:
    part = 'xl/' + target.lstrip('/').replace('xl/', '', 1) if not target.startswith('xl/') else target
    if part not in z.namelist():
        part = 'xl/' + target
    x = z.read(part).decode('utf8')
    fmt = re.search(r'<sheetFormatPr[^>]*>', x)
    defw = None
    base = 8
    if fmt:
        d = re.search(r'defaultColWidth="([\d.]+)"', fmt.group(0))
        b = re.search(r'baseColWidth="(\d+)"', fmt.group(0))
        if d: defw = (float(d.group(1)), BIAS)
        if b: base = int(b.group(1))
    if defw is None:
        defw = (base, BASE_PAD + BIAS)
    ours = {}
    hidden = set()
    colsec = re.search(r'<cols>(.*?)</cols>', x, re.S)
    if colsec:
        for cm in re.finditer(r'<col ([^>]*)/>', colsec.group(1)):
            a = cm.group(1)
            mn = int(re.search(r'min="(\d+)"', a).group(1))
            mx = int(re.search(r'max="(\d+)"', a).group(1))
            w = re.search(r'width="([\d.]+)"', a)
            hid = re.search(r'hidden="(1|true)"', a)
            ww = (float(w.group(1)), BIAS) if w else defw
            for c in range(mn, min(mx, 20000) + 1):
                ours[c] = ww
                if hid: hidden.add(c)
    lo = tables.get(name)
    if lo is None:
        print(f'{name}: no table in fods'); continue
    n = len(lo)
    bad = 0
    rows = []
    for i in range(n):
        c = i + 1
        w = ours.get(c, defw)
        o = at(*w)
        l = lo[i][0]
        if abs(o - l) > 0.51:
            bad += 1
        rows.append((c, o, l, o - l))
    print(f'--- {name}: {n} cols in fods, mismatched {bad}')
    for r in rows[:12]:
        print('    col%-4d ours %7d  lo %9.2f  d %+8.2f' % r)
