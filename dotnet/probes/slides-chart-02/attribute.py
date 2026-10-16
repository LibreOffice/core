#!/usr/bin/env python3
"""Which of the six changes moved which document.

Turns one change off at a time in the working tree, rebuilds the CLI, re-renders the eight
documents the before/after byte diff flagged, and reports which of them come out different
from the all-on build. Restores the tree after every variant.
"""
import subprocess, shutil, os, sys, pathlib

TREE = pathlib.Path('/c/sandbox/workdir/wt-s-chart/dotnet')
SAVE = pathlib.Path('/c/sandbox/workdir/s-chart-work/restore')
WORK = pathlib.Path('/c/sandbox/workdir/s-chart-work')
CLI = TREE / 'tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli'

FILES = {
    'plot': 'src/XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs',
    'layout': 'src/XRay.Office.Core/Charts/ChartLayout.cs',
    'chart': 'src/XRay.Office.Presentations/Ooxml/PptxSlideLayoutChart.cs',
}

VARIANTS = {
  'date-axis': ('plot',
    'ChartDateAxis? dateAxis = DateAxisOf(chartSpace, axes.Category, categoryValues);',
    'ChartDateAxis? dateAxis = null; _ = DateAxisOf(chartSpace, axes.Category, categoryValues);'),
  'dash': ('plot',
    '                DashPattern = DashOf(properties),',
    '                DashPattern = null,'),
  'legend-line-key': ('layout',
    '                if (entry.IsLine && entry.Line is { } sample)',
    '                if (false && entry.IsLine && entry.Line is { } sample)'),
  'percent-stacked': ('plot',
    '            IsPercentStacked = grouping is "percentStacked",',
    '            IsPercentStacked = false,'),
  'theme-override': ('chart',
    '        SlideTheme charted = ChartTheme(link.Target, theme);',
    '        SlideTheme charted = theme; _ = ChartTheme(link.Target, theme);'),
  'line-cap': ('plot',
    '                LineCap = CapOf(properties),',
    '                LineCap = XRay.Office.Core.Graphics.LineCap.Butt,'),
  'series-nofill': ('plot',
    '                SuppressesFill(properties) ? null : FillOf(properties, theme) ?? autoFill,',
    '                FillOf(properties, theme) ?? autoFill,'),
  'user-shapes': ('chart',
    '        drawn.AddRange(UserShapes(link.Target, slide, charted, local.Size, placement));',
    '        _ = UserShapes(link.Target, slide, charted, local.Size, placement);'),
}

DOCS = [l.strip() for l in open(WORK / 'reach-final.txt') if l.strip()]
CORPUS = pathlib.Path('/c/sandbox/workdir/sample-files')

def source_of(pdfname):
    stem = pdfname[:-len('__pptx.pdf')]
    for p in CORPUS.rglob('*'):
        if p.is_file() and p.stem == stem and p.suffix.lower() == '.pptx':
            return p
    raise SystemExit('no source for ' + pdfname)

SOURCES = {d: source_of(d) for d in DOCS}

def restore():
    for key, rel in FILES.items():
        shutil.copy(SAVE / pathlib.Path(rel).name, TREE / rel)

def build():
    r = subprocess.run(['dotnet', 'build', 'tools/XRay.Office.Cli/XRay.Office.Cli.csproj',
                        '-v', 'q', '-nologo', '--property:TreatWarningsAsErrors=false'],
                       cwd=TREE, capture_output=True, text=True)
    if r.returncode:
        print(r.stdout[-3000:]); raise SystemExit('build failed')

def render(outdir):
    outdir.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH='1755000000')
    for d, src in SOURCES.items():
        subprocess.run([str(CLI), 'render', str(src), '--outdir', str(outdir), '--quiet'],
                       env=env, capture_output=True)

restore(); build()
base = WORK / 'attr' / 'all-on'
render(base)

results = {}
for name, (key, old, new) in VARIANTS.items():
    restore()
    path = TREE / FILES[key]
    s = path.read_text()
    assert old in s, name
    path.write_text(s.replace(old, new))
    build()
    out = WORK / 'attr' / name
    render(out)
    moved = []
    for d, src in SOURCES.items():
        a = base / (src.stem + '.pdf')
        b = out / (src.stem + '.pdf')
        if not b.exists() or a.read_bytes() != b.read_bytes():
            moved.append(d)
    results[name] = moved
    print(name, '->', len(moved), 'documents')

restore(); build()

print()
print('change'.ljust(18), 'documents it moves')
for name, moved in results.items():
    print(name.ljust(18), ', '.join(sorted(x.replace('__pptx.pdf', '') for x in moved)) or '(none)')
