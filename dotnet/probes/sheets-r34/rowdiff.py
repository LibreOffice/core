#!/usr/bin/env python3
"""Our resolved row heights against LibreOffice's flat-ODF answer, per sheet.

Reports every differing row and, where the difference is a whole multiple of a plausible
line pitch, says so. Usage: rowdiff.py <document> <export.fods> [limit]
"""
import re, sys, subprocess

PROBE = ('/home/user/wt-sheets-r34/dotnet/probes/sheets-r33/GridProbe/'
         'bin/Debug/net10.0/linux-x64/GridProbe')
UNIT = {'in': 1440.0, 'cm': 1440 / 2.54, 'mm': 144 / 2.54, 'pt': 20.0}
LIMIT = int(sys.argv[3]) if len(sys.argv) > 3 else 4000


def unescape(name):
    return (name.replace('&lt;', '<').replace('&gt;', '>')
                .replace('&quot;', '"').replace('&apos;', "'").replace('&amp;', '&'))


s = open(sys.argv[2], encoding='utf-8').read()
styles = {}
for m in re.finditer(r'<style:style style:name="(ro\d+)"[^>]*>\s*'
                     r'<style:table-row-properties([^>]*)/>', s):
    h = re.search(r'style:row-height="([\d.]+)(in|cm|mm|pt)"', m.group(2))
    if h:
        styles[m.group(1)] = float(h.group(1)) * UNIT[h.group(2)]

tables = {}
for tm in re.finditer(r'<table:table table:name="([^"]*)"[^>]*>(.*?)(?=<table:table |\Z)', s, re.S):
    heights = []
    for rm in re.finditer(r'<table:table-row([^>]*)>', tm.group(2)):
        st = re.search(r'table:style-name="(ro\d+)"', rm.group(1))
        rep = re.search(r'table:number-rows-repeated="(\d+)"', rm.group(1))
        n = min(int(rep.group(1)) if rep else 1, LIMIT - len(heights))
        heights += [styles.get(st.group(1), 0.0) if st else 0.0] * max(0, n)
        if len(heights) >= LIMIT:
            break
    tables[unescape(tm.group(1))] = heights

out = subprocess.run([PROBE, sys.argv[1], '', '0', str(LIMIT)],
                     capture_output=True, text=True).stdout
name = None
total_rows = total_bad = 0
for line in out.splitlines():
    m = re.match(r"# sheet '(.*)' hidden=", line)
    if m:
        name = m.group(1)
        continue
    if not line.startswith('  rows ') or name is None:
        continue
    mine = [float(tok.split(':')[1].rstrip('H')) for tok in line[7:].split()]
    theirs = tables.get(name)
    if theirs is None:
        print(f'  {name!r}: no table in the export')
        continue
    n = min(len(mine), len(theirs))
    total_rows += n
    bad = [(i, mine[i], round(theirs[i], 1)) for i in range(n)
           if abs(mine[i] - theirs[i]) > 0.51]
    total_bad += len(bad)
    print(('  OK   ' if not bad else '  DIFF ') +
          f'{name!r}: {n} rows, {len(bad)} differ')
    for i, a, b in bad[:400]:
        print(f'      row {i:5d}  ours {a:8.1f}  ref {b:8.1f}  delta {a - b:+8.1f}')
print(f'# {total_rows} rows compared, {total_bad} differ')
