#!/usr/bin/env python3
"""What width 26.2.4.2 actually gives each part of every BIFF chart in the corpus.

`--convert-to ods` of a `.xls` puts each embedded chart in `Object N/content.xml`, where the
chart's own styles carry `svg:stroke-width` in centimetres.  This reads that back per chart
part, so the reach of the CHLINEFORMAT weight rule can be counted on the parts this tree
models -- axis, grid, series -- separately from the parts it does not: the chart frame and
the wall.
"""
import os, re, shutil, subprocess, sys, tempfile, zipfile
from collections import Counter

SOFFICE = '/opt/libreoffice26.2/program/soffice'
ROOT = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files/sheets'
NS_CH = re.compile(r'<chart:(\w+)([^>]*)>')


def widths(xml):
    # The properties element is NOT always the first child -- an axis and a series put
    # <style:chart-properties> ahead of <style:graphic-properties>, and a first cut of this
    # script that anchored on the first child reported "no axis or series states a width" for
    # the whole class, which is the opposite of the truth. Read the whole style block.
    styles = {}
    for m in re.finditer(r'<style:style style:name="([^"]+)".*?</style:style>', xml, re.S):
        body = m.group(0)
        w = re.search(r'svg:stroke-width="([^"]+)"', body)
        s = re.search(r'draw:stroke="([^"]+)"', body)
        styles[m.group(1)] = (w.group(1) if w else None, s.group(1) if s else None)
    out = []
    for m in NS_CH.finditer(xml):
        name = re.search(r'chart:style-name="([^"]+)"', m.group(2))
        if not name:
            continue
        got = styles.get(name.group(1))
        if got:
            out.append((m.group(1), got[0], got[1]))
    return out


def one(path):
    tmp = tempfile.mkdtemp(prefix='o61-')
    try:
        subprocess.run(['timeout', '-k', '30', '600', SOFFICE,
                        '-env:UserInstallation=file://' + tmp + '/p',
                        '--headless', '--norestore', '--convert-to', 'ods',
                        '--outdir', tmp, path], capture_output=True)
        got = [f for f in os.listdir(tmp) if f.endswith('.ods')]
        if not got:
            return None
        rows = []
        with zipfile.ZipFile(os.path.join(tmp, got[0])) as z:
            for n in z.namelist():
                if re.match(r'Object \d+/content\.xml$', n):
                    body = z.read(n).decode('utf-8', 'replace')
                    if 'chart:chart' in body:
                        rows += widths(body)
        return rows
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


docs = sorted(os.path.join(d, f)
              for d, _, fs in os.walk(ROOT) for f in fs
              if os.path.splitext(f)[1].lower() == '.xls')
print('path\tpart\twidth\tstroke')
for p in docs:
    rows = one(p)
    if not rows:
        continue
    for part, w, s in rows:
        print('%s\t%s\t%s\t%s' % (os.path.basename(p), part, w or '-', s or '-'))
