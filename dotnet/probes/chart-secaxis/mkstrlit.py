#!/usr/bin/env python3
"""Build two probes that separate a Calc limitation from a chart limitation.

The question is why 26.2.4.2 labels 029_Annual_budget's two columns `1` and `2` where the
chart part states `c:strLit` categories reading `Income` and `Expenses`. Two files, identical
chart XML, differing only in which application owns the chart:

  strlit.pptx   an Impress deck whose bar chart's c:cat is that same c:strLit
  strref.pptx   the same deck with the c:cat replaced by a c:strRef with a cached copy

If the deck draws the literal strings and the workbook does not, the limitation is Calc's
data provider and not the chart importer's.
"""
import re, shutil, sys, zipfile
from pathlib import Path

SRC = Path('/home/user/sample-files/slides/chartset-004/pptx/018_advanced_powerpoint_column.pptx')
OUT = Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/tmp-secaxis/strlit')

LIT = ('<c:cat><c:strLit><c:ptCount val="2"/>'
       '<c:pt idx="0"><c:v>Income</c:v></c:pt>'
       '<c:pt idx="1"><c:v>Expenses</c:v></c:pt></c:strLit></c:cat>')
REF = ('<c:cat><c:strRef><c:f>Sheet1!$A$2:$A$3</c:f><c:strCache><c:ptCount val="2"/>'
       '<c:pt idx="0"><c:v>Income</c:v></c:pt>'
       '<c:pt idx="1"><c:v>Expenses</c:v></c:pt></c:strCache></c:strRef></c:cat>')
VAL = ('<c:val><c:numLit><c:ptCount val="2"/>'
       '<c:pt idx="0"><c:v>4000</c:v></c:pt>'
       '<c:pt idx="1"><c:v>2476</c:v></c:pt></c:numLit></c:val>')

def build(tag, cat):
    OUT.mkdir(parents=True, exist_ok=True)
    target = OUT / f'{tag}.pptx'
    zin = zipfile.ZipFile(SRC)
    part = next(n for n in zin.namelist() if re.match(r'ppt/charts/chart\d+\.xml$', n))
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == part:
                x = data.decode('utf-8')
                # one series only, with the categories and values this probe states
                x = re.sub(r'<c:ser>.*</c:ser>',
                           '<c:ser><c:idx val="0"/><c:order val="0"/>' + cat + VAL + '</c:ser>',
                           x, flags=re.S)
                data = x.encode('utf-8')
            zout.writestr(item, data)
    return target

for tag, cat in (('strlit', LIT), ('strref', REF)):
    print(build(tag, cat))
