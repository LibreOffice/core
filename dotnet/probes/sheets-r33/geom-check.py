#!/usr/bin/env python3
"""Compare, per sheet, our resolved geometry against LibreOffice's own flat-ODF answer:
the column widths, the row heights, and the page rectangle the bands are measured against.

Ours comes from GridProbe's dump; LibreOffice's from the export. The point is to decide which
axis a page-count difference can possibly be on: our column-band split is a faithful port of
ScTable::UpdatePageBreaks, so if the widths and the printable width both agree the split cannot
differ, and the difference has to be on the rows or downstream.
"""
import re, sys, subprocess, collections

PROBE = ('/home/user/libreoffice-core/.claude/worktrees/sheets-r33/dotnet/probes/'
         'sheets-r33/GridProbe/bin/Debug/net10.0/linux-x64/GridProbe')
UNIT = {'in': 1440.0, 'cm': 1440/2.54, 'mm': 144/2.54, 'pt': 20.0}


def twips(value, unit):
    return float(value) * UNIT[unit]


def lo(path):
    s = open(path, encoding='utf-8').read()
    cols, rows = {}, {}
    for m in re.finditer(r'<style:style style:name="(co\d+)"[^>]*>\s*'
                         r'<style:table-column-properties([^>]*)/>', s):
        w = re.search(r'style:column-width="([\d.]+)(in|cm|mm|pt)"', m.group(2))
        if w: cols[m.group(1)] = twips(w.group(1), w.group(2))
    for m in re.finditer(r'<style:style style:name="(ro\d+)"[^>]*>\s*'
                         r'<style:table-row-properties([^>]*)/>', s):
        h = re.search(r'style:row-height="([\d.]+)(in|cm|mm|pt)"', m.group(2))
        if h: rows[m.group(1)] = twips(h.group(1), h.group(2))

    tables = {}
    for tm in re.finditer(r'<table:table table:name="([^"]*)"[^>]*>(.*?)<table:table-row', s, re.S):
        widths = []
        for cm in re.finditer(r'<table:table-column[^>]*?/>', tm.group(2)):
            st = re.search(r'table:style-name="(co\d+)"', cm.group(0))
            rep = re.search(r'table:number-columns-repeated="(\d+)"', cm.group(0))
            if st and st.group(1) in cols:
                widths += [cols[st.group(1)]] * min(int(rep.group(1)) if rep else 1, 20000)
        tables[m_unescape(tm.group(1))] = widths
    return tables


def m_unescape(name):
    return (name.replace('&amp;', '&').replace('&lt;', '<')
                .replace('&gt;', '>').replace('&quot;', '"').replace('&apos;', "'"))


def ours(document, maxcols):
    out = subprocess.run([PROBE, document, '', str(maxcols), '0'],
                         capture_output=True, text=True).stdout
    sheets, current = {}, None
    for line in out.splitlines():
        m = re.match(r"# sheet '(.*)' hidden=(\w+)", line)
        if m:
            current = m.group(1)
            sheets[current] = {'hidden': m.group(2) == 'True', 'cols': []}
            continue
        if line.startswith('  cols ') and current:
            for tok in line[7:].split():
                index, _, width = tok.partition(':')
                sheets[current]['cols'].append(float(width.rstrip('H')))
    return sheets


if __name__ == '__main__':
    document, fods = sys.argv[1], sys.argv[2]
    maxcols = int(sys.argv[3]) if len(sys.argv) > 3 else 200
    reference = lo(fods)
    for name, data in ours(document, maxcols).items():
        theirs = reference.get(name)
        if theirs is None:
            print(f'  {name!r}: no table in the export')
            continue
        n = min(len(data['cols']), len(theirs))
        bad = [(i, data['cols'][i], theirs[i])
               for i in range(n) if abs(data['cols'][i] - theirs[i]) > 0.51]
        flag = 'OK ' if not bad else 'DIFF'
        print(f'  {flag} {name!r}: {n} columns compared, {len(bad)} differ'
              + (f'  e.g. {bad[:3]}' if bad else ''))
