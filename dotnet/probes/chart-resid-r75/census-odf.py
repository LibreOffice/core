#!/usr/bin/env python3
"""Which files of the converted ODF corpus embed a chart, and which of those hold a line
or scatter series — the only kind `ChartClipping` can reach."""
import pathlib, sys, zipfile
root = pathlib.Path('/home/user/corpus-odf')
rows = []
for p in sorted(root.rglob('*')):
    if not p.is_file() or p.suffix.lower() not in ('.odt', '.ods', '.odp', '.rtf'):
        continue
    if p.suffix.lower() == '.rtf':
        continue          # RTF cannot carry an ODF chart object
    try:
        z = zipfile.ZipFile(p)
    except Exception:
        continue
    names = [n for n in z.namelist() if n.endswith('content.xml')]
    charts = 0; lines = 0
    for n in names:
        try: b = z.read(n)
        except Exception: continue
        if b'chart:chart' not in b: continue
        charts += 1
        if b'chart:class="chart:line"' in b or b'chart:class="chart:scatter"' in b:
            lines += 1
    if charts:
        rows.append((str(p.relative_to(root)), charts, lines))
print('path\tcharts\tlineparts')
for r in rows: print('%s\t%d\t%d' % r)
print('# files with a chart: %d, of which line/scatter: %d' % (
    len(rows), sum(1 for r in rows if r[2])), file=sys.stderr)
